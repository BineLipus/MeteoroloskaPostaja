<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Meteorological Station Leepush</title>
    <script src="http://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script>
    <!-- Za AJAX poizvedbo (easy način) rabimo JQuery -->
    <style>
        table {
            font-family: arial, sans-serif;
            border-collapse: collapse;
            width: 100%;
        }

        td, th {
            border: 1px solid #dddddd;
            text-align: left;
            padding: 8px;
        }

        tr:nth-child(even) {
            background-color: #dddddd;
        }
    </style>
    <script>
        function send_AJAX_request() {
            // Rabimo JQuery
            $.post("AJAX_Handler.php",
                function (data) {
                    var tableHeaderHTML = document.getElementById("header").outerHTML;
                    data = tableHeaderHTML + data;
                    $("table").html(data);
                }
            );
        }

        $(document).ready(() => {
            // Na začetku takoj pokažemo, nato vsakih x sekundo posodobimo podatke.
            send_AJAX_request();
            timer = setInterval(function () {
                console.log("AJAX request call -- Updating data");
                send_AJAX_request(); // Sends AJAX request every minute.
            }, 30000);

        });
    </script>
</head>
<body>
<table>
    <tr id="header">
        <th>Date & Time</th>
        <th>Temperature</th>
        <th>Humidity</th>
        <th>Pressure</th>
    </tr>
</table>
</body>