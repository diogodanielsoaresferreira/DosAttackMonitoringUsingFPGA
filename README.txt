-- NetworkMonitoring_sources
Grande parte dos ficheiros *.c são gerados automaticamente pelo Projeto do Lightweight IP.

Os ficheiros criados por nós são:
* main.c - Configura e efetua a inicialização dos componentes necessários.
* receivePackets.c - Recebe os pacotes da interface ethernet.
* processPackets.h/processPackets.c - Contêm as funções necessárias ao processamento de pacotes e às atualizações necessárias nos contadores.
* MemoryUtils.h/MemoryUtils.c - Funções de escrita e leitura da memória e cache, através de CDMA.
* BloomFilterUtils.h/BloomFilterUtils.c - Funções de escrita e leitura no Bloom Filter através do periférico.


-- DisplayController
Controlador de displays efetuado na aula, onde o processador controla o valor a ser enviado para os displays.


-- RGB_LED_Controller
Controlador de led RGB efetuado na aula, onde é possível especificar a percentagem (de 0 a 255) de cada cor e o brilho.


-- BloomFilterStream
Periférico de streaming que implementa as hashes e os bloom filters.
O processador altera um registo memory-mapped para indicar se deseja efetuar uma leitura ou escrita, e envia os dados na interface de streaming.

A função de hash foi implementada em https://github.com/pemb/siphash.
O controlo da função de hash e o bloom filter foi implementado por nós, numa máquina de estados.

É também possível efetuar um reset global através de um registo memory-mapped. Como a frequência máxima de operação é 44.65 Mhz, é necessário um clock mais lento. Para a transferência de dados, é necessário que existam dois Clock Converters entre o periférico e o processador, para as entradas/saídas.
