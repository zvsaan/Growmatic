const express = require('express');
const mysql = require('mysql');
const bodyParser = require('body-parser');
const cors = require('cors');

const app = express();
app.use(bodyParser.json());
app.use(cors());

// Buat koneksi ke database MySQL
const db = mysql.createConnection({
  host: 'localhost',
  user: 'root',
  password: '',
  database: 'sensor_db'
});

db.connect((err) => {
  if (err) {
    throw err;
  }
  console.log('MySQL Connected...');
});

// Endpoint untuk menyimpan data sensor
app.post('/save-data', (req, res) => {
  const { temperature, humidity } = req.body;

  const sql = 'INSERT INTO sensor_data (temperature, humidity) VALUES (?, ?)';
  db.query(sql, [temperature, humidity], (err, result) => {
    if (err) {
      console.error('Error saving data to database:', err);
      return res.status(500).json({ error: 'Failed to save data' });
    }
    console.log('Data saved to database:', result);
    res.status(200).json({ message: 'Data saved successfully' });
  });
});

// Endpoint untuk mengambil data suhu dan kelembapan
app.get('/get-data', (req, res) => {
  const sql = 'SELECT temperature, humidity, timestamp FROM sensor_data ORDER BY timestamp DESC LIMIT 100'; // Ambil 100 data terbaru
  // const sql = 'SELECT temperature, humidity FROM sensor_data LIMIT 100';
  db.query(sql, (err, results) => {
    if (err) {
      console.error('Error fetching data from database:', err);
      return res.status(500).json({ error: 'Failed to fetch data' });
    }
    res.status(200).json(results);
  });
});



const server = app.listen(3000, () => {
  console.log(`Server running on port 3000`);
});
