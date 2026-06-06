const express = require("express")
const app = express()
const port = 3000
var path = require("path")
const fs = require("fs")

// We use an ejs File to insert the randomly chosen content.
app.set("view engine", "ejs");
app.set("views", path.join(__dirname, "public/views"));

app.use(express.static("public"))

function applyZeros(numbers, zeros) {
  const arr = numbers.split('');
  const indices = new Set();
  while (indices.size < zeros) {
    const randomIndex = Math.floor(Math.random() * 81);
    indices.add(randomIndex);
  }
  indices.forEach(i => { arr[i] = '0'; });
  return arr;
}

const ZEROS = 40;

app.get(["/"], async (req, res) => {
  const text = await fetch("http://127.0.0.1:8080/")
      .then((response) => {
        if (!response.ok) {
          throw new Error(`HTTP error: ${response.status}`);
        }
        return response.text();
      }).then((text) => {
        return text;
      });
  const sudoku_text= text.split('&')[0].split('=')[1];
  const time = text.split('&')[1].split('=')[1];
  const numbers = applyZeros(sudoku_text, ZEROS);
  res.render("frontend", { numbers, time });
})

app.get("/check", async (req, res) => {
    const board = req.query.board;
    if (!board || board.length !== 81) {
        return res.status(400).send("Invalid board");
    }
    try {
        const response = await fetch(`http://127.0.0.1:8080/?board=${board}`);
        const text = await response.text();
        res.send(text);
    } catch (err) {
        res.status(500).send("Backend server error");
    }
});

app.listen(port, () => {
  console.log(`sudoku frontend listening on port ${port}`)
})
