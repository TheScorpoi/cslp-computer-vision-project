# cslp-computer-vision-project

O objetivo do projeto é desenvolver software capaz de detetar peças de LEGO, a sua cor, e o seu tamanho.    
O software é capaz de reconhecer mulitplas peças de tamanho variável, bem como de várias cores diferentes, em tempo real.
A maneira implementada para reconhecer a cor das peças, é verificar se uma determinada região (entenda-se por região, um conjunto alargado de pixeis com valores semelhantes), pertence a alguma cor definida.
Para determinar o tamanho da peça, está é feita através de calculos com a largura e o comprimento dos contornos que a peça possui.  
O projeto foi todo ele desenvolvido para correr em tempo real num Raspberry Pi 4, com recurso a uma câmara instalada no mesmo.


## Pre-requisitos:
Relativamente aos pre-requisitos, é necessário:
- Instalação de um compilador de [C++](https://linuxconfig.org/how-to-install-g-the-c-compiler-on-ubuntu-18-04-bionic-beaver-linux).
- Instalação do [CMake](https://cgold.readthedocs.io/en/latest/first-step/installation.html).
- A biblioteca do [OpenCV](https://docs.opencv.org/4.x/df/d65/tutorial_table_of_content_introduction.html).
- Instalação da biblioteca *boost* para C++: ``` 
sudo apt-get install libboost-all-dev ```
- Raspberry Pi 4 (para correr em tempo real o códgo desenvolvido).


## Como correr:
Para correr o projeto, é preciso seguir os passos abaixo:
- Entrar no diretorio src/build/:
```bash
cd src/build/
```
- Correr o CMake:
```bash
cmake ..
```
- Compilar o programa em C++:
```bash
make
```
- Correr o executável gerado pela compilação:
```bash
./ComputerVision [opção]
```
> Quanto à opção, esta pode tomar as seguintes letras:
> 
> - **-n** : O programa é executado e identifica 4 cores pre-definidas (azul, vermelho, amarelo, verde).
> - **-c** : Calibração da camara.
> - **-a** : Adição de novas cores para a deteção.
> - **-e** : Leitura de um ficheiro com informações sobre cores, para a deteção.
>
>>Ficheiro apresenta a seguinte estrutura:
>> - [Nome da cor] -  [HSV mínimo] -  [HSV máximo] - [RGB da cor]


## Implementação
O projeto está desenvolvido com a seguinte fluxo:

- 1. Captação de imagem.
- 2. Aplicação de um filtro de Blur.
- 3. Converção do espaço de cor BGR para HSV.
- 4. Utilização da função *inRange* para detetar se as cores da imagem captada se encontram dentro do intervalo de cores definidas.
- 5. Aplicação de operadores morfológicos - *erode()* e *dilate()*.
- 6. Utilização da função *findContours* para encontrar os contornos da imagem, e marcação dos pixeis onde será feito o contorno da peça.
- 7. Desenho do contorno na janela de video, com a cor, a forma e o tamanho da peça.

