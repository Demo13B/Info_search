import axios from 'axios'
import zlib from 'zlib'
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
                "Accept": "*/*",
                "Referer": "https://www.motorsport.com/sitemaps/news.xml"
            },
        }
    );


    let xml;
    if (sitemap.endsWith('.gz')) {
        xml = zlib.gunzipSync(res.data).toString();
    } else {
        xml = res.data.toString();
    }

    const index = await parser.parse(xml);
    const links = index.urlset.url
        .map(url => url.loc)
        .filter(url => url.includes('/f1/news'));
    return links;
}

async function sitemapSweeper() {
    const sitemap = 'https://www.motorsport.com/sitemap.xml'

    const res = await axios.get(
        sitemap,
        {
            headers: {
                "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36",
                "Accept": "*/*",
                "Referer": "https://www.motorsport.com/sitemaps/news.xml"
            },
        }
    );

    const xml = await parser.parse(res.data)

    const sub_sitemaps = xml.sitemapindex.sitemap.map(s => s.loc).filter(s => s.includes('sm_articles') || s.includes('news.xml'));

    let article_urls = []
    for (let sub_sitemap of sub_sitemaps) {
        const response = await url_collector(sub_sitemap);
        article_urls.push(...response)
        console.log(`Downloaded list from: ${sub_sitemap}`);
        console.log(`Total articles: ${article_urls.length}`)
    }

    return article_urls;
}

async function main() {
    const urls = await sitemapSweeper();
    fs.writeFileSync('motorsport_urls.json', JSON.stringify(urls, null, 2), 'utf-8');
    console.log('---------------------------------');
    console.log('Urls written into file');
    console.log('---------------------------------');
}

main();