const express = require('express');
const sqlite3 = require('sqlite3');
const app = express();

app.get('/api/stream-key', (req, res) => {
    const db = new sqlite3.Database('/var/www/pongrobot-backend/pongrobot.db');
    db.get('SELECT key FROM users WHERE balls > 0 LIMIT 1', (err, row) => {
        if (err || !row) {
            return res.status(403).json({ error: 'No valid stream key' });
        }
        res.json({ streamKey: row.key });
    });
    db.close();
});

app.listen(3000, () => console.log('Backend running on port 3000'));
