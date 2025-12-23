import axios from 'axios'
import { XMLParser } from 'fast-xml-parser'
import fs from 'fs'

const parser = new XMLParser();

async function url_collector(sitemap) {
    const res = await axios.get(
        sitemap,
        {
            responseType: 'arraybuffer',
            headers: {
                "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
                "Accept": "*/*"
            },
        }
    );

    const xml = await parser.parse(res.data.toString());
    const links = xml.urlset.url.map(url => url.loc).filter(url => url.includes('/formula-1/'));

    return links;
}

async function main() {
    const urls = await url_collector('https://www.the-race.com/sitemap-posts.xml')
    fs.writeFileSync('the_race_urls.json', JSON.stringify(urls, null, 2), 'utf-8');
    console.log('---------------------------------');
    console.log('Urls written into file');
    console.log('---------------------------------');
}

main()