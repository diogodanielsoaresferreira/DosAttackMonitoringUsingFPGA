# DoS Attack Monitoring using an FPGA

Project presented in the Reconfigurable Computation course (Universidade de Aveiro), 2018.

FPGA that detects DoS attacks to a server and blocks the correspondent IPs.
When the packets are received, the FPGA uses concurrently several Bloom Filters to check in real-time if the IP is blocked.

The implemented hash-function is https://github.com/pemb/siphash.

Diogo Ferreira
Luís Leira
