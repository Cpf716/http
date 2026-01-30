// Corey Ferguson
// September 24, 2025
// http
// index.js
//

fetch('http://localhost:8080/api/ping').then(response =>
    response.text().then(value => console.log(value))
).catch(err => {
    throw err;
})