# HTTP Server (C++)

Thanks for checking out my C++ HTTP server!

It is a crude implementation of the HTTP protocol down to the socket layer. While more powerful and widely adopted implementations exist, this one gives you an opportunity to see the protocol in action at a low level.

## Requirements

Node v18 or higher, Visual Studio Code (VS Code), and Xcode

## Build and Run

### Server

Open the project in Xcode and select <i>Product > Run</i> from the toolbar or press `Command + R` on the keyboard

You should see the following output:
```
Server listening on port 8080...
```

<b>You can change the port number through the `PORT` constant in `http/src/main.cpp`</b>

### Client

Open the project in VS Code, open the integrated Terminal, and run the following command:
```
node http/src/index.js
```

You should see the following output:
```
Hello, world!
```

## Examples

Optionally run the test server for my other project [fetch](https://github.com/Cpf716/fetch):

Replace the contents of `http/src/index.js` with the following:
```
fetch('http://localhost:8080/greeting', {
    method: 'post',
    body: JSON.stringify({
        firstName: "Corey",
        // lastName: "Ferguson",
        // fullName: "Corey Ferguson"
    }),
    headers: {
        'Content-Type': 'application/json'
    }
}).then(response =>
    response.text().then(value => console.log(value))
).catch(err => {
    throw err;
});
```
Run the following command in VS Code's integrated terminal:
```
node http/src/index.js
```

You should see the following output:
```
Happy Wednesday, Corey!
```