<?php
header('Content-Type: text/html');
?>
<!DOCTYPE html>
<html>
<head>
    <title>Psychedelic Page</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            font-family: Arial, sans-serif;
            color: #fff;
            /* Psychedelic animated background gradient */
            background: linear-gradient(45deg, #ff0000, #00ff00, #0000ff, #ffff00, #ff00ff, #00ffff);
            background-size: 600% 600%;
            animation: gradientAnimation 10s ease infinite;
        }

        @keyframes gradientAnimation {
            0%   { background-position: 0% 50%; }
            50%  { background-position: 100% 50%; }
            100% { background-position: 0% 50%; }
        }

        h1 {
            text-align: center;
            margin-top: 100px;
            font-size: 4em;
            text-shadow: 0 0 5px #fff, 0 0 10px #fff;
        }

        p {
            text-align: center;
            font-size: 1.5em;
            margin-top: 50px;
            text-shadow: 0 0 5px #fff;
        }
    </style>
</head>
<body>
    <h1>Welcome to the Psychedelic Page!</h1>
    <p>Enjoy the vivid swirling colors and let your mind drift...</p>
</body>
</html>