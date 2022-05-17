<?php
$conn = null;
function connectToDB()
{
    global $conn;
    $servername = "server_domain_or_IP";
    $username = "db_user";
    $password = "db_password";
    $db_name = "arduino_meteorological_station";
    $port = 3306;

    // Create connection
    $conn = new mysqli($servername, $username, $password, $db_name, $port);
    // Check connection
    return !($conn->connect_error); // Sporočimo uspeh
}

function disconnectFromDB()
{
    global $conn;
    if (isset($conn)) $conn->close();

}

function select()
{
    global $conn;
    if (!connectToDB()) return false; // Če connectiona ni, sporočimo neuspeh

    // Sprintf je printf, katere rezultat je String in ne izpis
    $sql = "SELECT * FROM measurement ORDER BY recorded DESC;";
    $result = $conn->query($sql);

    if ($result->num_rows > 0) {
        // output data of each row
        while ($row = $result->fetch_assoc()):
            ?>
            <tr>
                <td><?= $row["recorded"] ?></td>
                <td><?= $row["temperature"] ?> &#8451;</td>
                <td><?= $row["humidity"] ?>%</td>
                <td><?= $row["pressure"] ?> bar</td>
            </tr>
        <?php
        endwhile;
    }

    disconnectFromDB();
}