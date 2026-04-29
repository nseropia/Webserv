<?php
// header('Content-Type: text/plain');

if (isset($_GET['name'])) {
    echo "Hello, " . htmlspecialchars($_GET['name']) . "!";
} else {
    echo "Hello, World!";
}
?>