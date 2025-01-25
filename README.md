# System Dashboard

A system dashboard that monitors system resources and provides system information.

![System Dashboard Screenshot](assets/Screenshot.png)

Made with my own use case in mind, all made from scratch, the program is a server and a html parser.

## Features

On the web interface:
- Display CPU usage
- Display RAM usage
- Display Disk usage
- Display system temperature
- View and manage services
- View program logs

On the server:
- Choose the port
- View program logs

## Getting Started

### Prerequisites

- GCC compiler
- Make
- Gengetopt

### Building the Project

To build the project, run the following command:

```sh
make
```

### Running the Project

To run the project, execute the following command:

```sh
./sysDash
```

### Configuration

You can configure the server port by using the `--port` option:

```sh
./sysDash --port 8080
```

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
