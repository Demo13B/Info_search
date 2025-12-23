import redis from 'redis'
import { MongoClient } from 'mongodb'
import yaml from 'js-yaml'
import fs from 'fs'
import axios from 'axios'
import zlib from 'zlib'

class RedisQueue {
    constructor(connection_url) {
        this.connection_url = connection_url;
        this.client = redis.createClient({
            url: connection_url
        });
    }

    async connect() {
        await this.client.connect();
    }

    async disconnect() {
        this.client.destroy();
    }

    async addToPending(urls) {
        for (const url of urls) {
            await this.client.lPush('queue:pending', url);
        }
    }

    async startProcessing() {
        const url = await this.client.rPopLPush('queue:pending', 'queue:processing');
        return url;
    }

    async endProcessing() {
        await this.client.rPop('queue:processing');
    }

    async getProcessing() {
        return (await this.client.lRange('queue:processing', -1, -1))[0];
    }

    async processingIsEmpty() {
        const len = await this.client.lLen('queue:processing');
        return len === 0;
    }

    async pendingIsEmpty() {
        const len = await this.client.lLen('queue:pending');
        return len === 0;
    }
};

class MongoDB {
    constructor(connection_url, db_name, collection_name) {
        this.connection_url = connection_url;
        this.client = new MongoClient(connection_url);
        this.db_name = db_name;
        this.collection_name = collection_name;
    };

    async connect() {
        await this.client.connect()
        this.collection = this.client.db(this.db_name).collection(this.collection_name);
        await this.collection.createIndex({ url: 1 }, { unique: true });
    }

    async disconnect() {
        await this.client.close();
    }

    async insertDocument(url, data, source) {
        try {
            await this.collection.insertOne({
                url: url,
                data: zlib.gzipSync(data),
                source: source,
                date: Date.now()
            });
        } catch (e) {
            throw (e);
        }
    }

    async findDocument(url) {
        try {
            const document = await this.collection.findOne({ url: url });
            return document;
        } catch (e) {
            throw (e);
        }
    }

    async updateDocument(url, newData) {
        try {
            await this.collection.updateOne(
                { url: url },
                {
                    $set: {
                        data: zlib.gzipSync(newData),
                        date: Date.now()
                    }
                }
            );
        } catch (e) {
            throw e;
        }
    }

    async getCursor() {
        let cursor;
        try {
            cursor = this.collection.find({});
        } catch (e) {
            throw e;
        }

        return cursor;
    }
};

class CrawlerBot {
    constructor(db, queue) {
        this.db = db;
        this.queue = queue;
    }

    async delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    async poopulationMode(sources, delay) {
        if (await this.queue.pendingIsEmpty()) {
            console.log('Updating pending.....');
            for (let source of sources) {
                await this.queue.addToPending(source);
            }
        }

        console.log('Retrying failed');
        while (!(await this.queue.processingIsEmpty())) {
            let url;
            try {
                url = await this.queue.getProcessing();
            } catch (e) {
                console.error(e);
                return;
            }

            const res = await axios.get(
                url,
                {
                    headers: {
                        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
                        "Accept": "*/*"
                    },
                }
            );

            if (res.status != 200) {
                console.error(res);
                return;
            } else {
                try {
                    if (await this.db.findDocument(url)) {
                        console.log('Document already in db.... Skipping');
                        await this.queue.endProcessing();
                        continue;
                    }
                    await this.db.insertDocument(url, res.data, new URL(url).hostname);
                } catch (e) {
                    console.error(e);
                    return;
                }

                await this.queue.endProcessing();
            }
            await this.delay(delay);
        }
        console.log('Retrying done');


        while (true) {
            let url;
            try {
                url = await this.queue.startProcessing();
            } catch (e) {
                console.error(e);
                return;
            }

            if (url === null) {
                console.log('----------------------------');
                console.log('Queue is empty... Stopping');
                break;
            }

            console.log('-------------------------');
            console.log(`Starting download from: ${url}`)

            const res = await axios.get(
                url,
                {
                    headers: {
                        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
                        "Accept": "*/*"
                    },
                }
            );

            if (res.status != 200) {
                console.error(res);
                return;
            } else {
                try {
                    if (await this.db.findDocument(url)) {
                        console.log('Document already in db.... Skipping');
                        await this.queue.endProcessing();
                        continue;
                    }
                    await this.db.insertDocument(url, res.data, new URL(url).hostname);
                } catch (e) {
                    console.error(e);
                    return;
                }

                await this.queue.endProcessing();

                console.log(`Status: 200. Inserted into database`);
            }
            await this.delay(delay);
        }
    }

    async updateMode(delay) {
        console.log('Starting database sweep');

        const cursor = await this.db.getCursor();

        while (await cursor.hasNext()) {
            const document = await cursor.next();

            console.log('--------------------------------------');
            console.log(`Checking at: ${document.url}`);

            if (Date.now() - document.date < 7 * 24 * 60 * 60 * 1000) {
                console.log('The document is fresh');
                continue;
            }

            const res = await axios.get(
                document.url,
                {
                    headers: {
                        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
                        "Accept": "*/*"
                    },
                }
            );

            if (res.status != 200) {
                console.error(res);
                return;
            }

            const newCompressed = zlib.gzipSync(res.data);

            if (document.data == newCompressed) {
                console.log("The document hasn't changed");
            } else {
                console.log('Updating document');
                await this.db.updateDocument(document.url, res.data);
            }

            await this.delay(delay);
        }
    }
};

async function main() {
    const config_path = process.argv[2];
    const config_raw = fs.readFileSync(config_path, 'utf-8');
    const config = yaml.load(config_raw);

    if (!config.mongo || !config.redis) {
        console.error('Wrong config file format');
        return;
    } else if (!config.mode) {
        console.error('No mode specified');
        return;
    } else if (!config.sources) {
        console.error('No sources provided');
        return;
    }

    const db = new MongoDB(config.mongo.url, config.mongo.db_name, config.mongo.collection_name);
    const queue = new RedisQueue(config.redis.url);

    try {
        await db.connect();
        await queue.connect();
    } catch (e) {
        console.error(e);
        return;
    }
    console.log('Connection to database and queue established');

    let links = [];
    for (let source of config.sources) {
        const raw = fs.readFileSync(source, 'utf-8');
        links.push(JSON.parse(raw));
    }

    const crawler = new CrawlerBot(db, queue);

    if (config.mode === 'populate') {
        await crawler.poopulationMode(links, 100);
    } else if (config.mode === 'update') {
        await crawler.updateMode(100);
    } else {
        console.error('Wrong mode');
    }

    db.disconnect();
    queue.disconnect();
    console.log('Database and queue disconnected');
}

main();