const http = require('http');
const httpProxy = require('http-proxy');

const proxy = httpProxy.createProxyServer({});

const server = http.createServer((req, res) => {
    res.setHeader('Content-Security-Policy', "default-src 'self'; script-src 'self' 'unsafe-inline'; connect-src 'self' ws://192.168.1.222:81 http://192.168.1.229:4747 http://192.168.1.81:4747;");

    const target = req.url.startsWith('/stream1') ? 'http://192.168.1.229:4747/video' : 'http://192.168.1.81:4747/video';
    proxy.web(req, res, { target });
});

server.listen(3000, () => {
    console.log('Proxy server is running on http://localhost:3000');
});

