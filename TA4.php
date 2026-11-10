<?php
header("Content-Type: application/json");

$users = [
    ["id" => 1, "name" => "Alice"],
    ["id" => 2, "name" => "Bob"],
];

if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    echo json_encode($users);
} elseif ($_SERVER['REQUEST_METHOD'] === 'POST') {

    $data = json_decode(
        file_get_contents("php://input"),
        true
    );

    $newUser = [
        "id" => count($users) + 1,
        "name" => $data['name']
    ];
    $users[] = $newUser;
    
    echo json_encode($newUser);
}
?>