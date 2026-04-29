<?php
header('Content-Type: text/html; charset=UTF-8');
// Check if the request method is POST
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Read the raw input from the request body
    $user_message = (isset($_POST['message']) && trim($_POST['message']) !== '') ? $_POST['message'] : 'No message given!';

    // Print a psychedelic HTML page
    echo "<!DOCTYPE html>";
    echo "<html lang='en'>";
    echo "<head>";
    echo "    <meta charset='UTF-8'>";
    echo "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    echo "    <title>Psychedelic POST Portal</title>";
    echo "    <style>";
    echo "        body {";
    echo "            background: linear-gradient(135deg, #FF00FF, #00FFFF, #FFFF00, #FF00FF);";
    echo "            background-size: 400% 400%;";
    echo "            animation: psychedelic 10s infinite alternate ease-in-out;";
    echo "            color: #FFF;";
    echo "            font-family: 'Courier New', monospace;";
    echo "            text-align: center;";
    echo "            padding: 50px;";
    echo "        }";
    echo "        @keyframes psychedelic {";
    echo "            0% { background-position: 0% 50%; }";
    echo "            50% { background-position: 100% 50%; }";
    echo "            100% { background-position: 0% 50%; }";
    echo "        }";
    echo "        h1 {";
    echo "            font-size: 3em;";
    echo "            text-shadow: 2px 2px #000;";
    echo "            margin-bottom: 1em;";
    echo "        }";
    echo "        .message-box {";
    echo "            background: rgba(0, 0, 0, 0.3);";
    echo "            border: 3px dashed #FFF;";
    echo "            display: inline-block;";
    echo "            padding: 20px;";
    echo "            animation: spin 8s infinite linear;";
    echo "        }";
    echo "        @keyframes spin {";
    echo "            from {transform: rotate(0deg);}";
    echo "            to {transform: rotate(360deg);}";
    echo "        }";
    echo "    </style>";
    echo "</head>";
    echo "<body>";
    echo "    <h1>Welcome to the POST Portal!</h1>";
    echo "    <div class='message-box'>";
    echo "        <p>You've transmitted a cosmic message through the swirling vortex:</p>";
    echo "        <p><h1><strong>" . htmlspecialchars($user_message) . "</strong></h1></p>";
    echo "        <p>Groovy vibrations are heading your way!</p>";
    echo "    </div>";
    echo "</body>";
    echo "</html>";
} else {
    // If it's not a POST request, show a simple message
    echo "<!DOCTYPE html>";
    echo "<html>";
    echo "<head><title>Method Not Allowed</title></head>";
    echo "<body style='background: #000; color: #FFF; font-family: monospace; text-align: center;'>";
    echo "<h1>Hey, buddy, you gotta POST raw data to get this trip!</h1>";
    echo "</body>";
    echo "</html>";
}
?>