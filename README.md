# Descrição

O objetivo deste projeto é criar uma função em C que, quando chamada dentro de um loop, permita a leitura de um arquivo disponível através de um file descriptor (fd), uma linha por vez até o seu fim. O principal desafio aqui não é apenas ler o arquivo, mas garantir que se um bloco de dados (Buffer) contiver mais de uma linha, o excesso seja preservado e reutilizado na próxima vez que a função for chamada através de uma variável estática (static).

### Objetivos Principais
* Compreender e aplicar o conceito de variáveis estáticas.
* Gerenciamento rigoroso de memória (alocação dinâmica e desalocação via malloc/free).
* Manipulação de arquivo através das chamadas de sistema open() e read().

# Instruções

### Compilação

Para compilar o projeto, certifique-se de ter um compilador C instalado. Caso você não tenha, siga o passo a passo abaixo:

#### Debian e derivados (Ubuntu, Linux Mint, Kali Linux, etc):
`sudo apt update`
`sudo apt install build-essential`

#### Fedora, RHEL e CentOS:
`sudo dnf groupinstall "Development Tools"` 

#### Arch Linux:
`sudo pacman -S base-devel`

Após instalar o compilador, execute o seguinte comando no seu terminal:
`gcc -Wall -Wextra -Werror -D BUFFER_SIZE=10 get_next_line.c get_next_line_utils.c -o get_next_line`

Observação: o valor do BUFFER_SIZE fica ao seu critério.

### Execução

Para executar o programa basta alterar o texto do arquivo text.txt com um texto que você desejar e depois rodar o comando:
`./get_next_line`

você precisará de uma `main` para criar um loop onde o arquivo será lido até o final, mas não se preocupe pois essa main já está criada, caso tenha curiosidade em saber como é essa main, basta abrir o arquivo `get_next_line.c`

# Explicação

### O que de é e para que serve o File Descriptor (FD)? 
O que você precisa entender sobre o FD é que ele não é o seu arquivo lido literalmente, mas sim uma referência em um índice para o seu arquivo.
O FD é a forma como a linguagem C interage com as chamadas de sistema (System calls ou Syscall). Imagino que deve ter ficado um pouco obscuro caso você não saiba o que são Syscall. Resumindo, Syscall são chamadas que atuam como uma interface essencial entre as aplicações (o seu código) e o sistema operacional (mas especificamente com o Kernel, que é um intermediário entre o software e o hardware), fornecendo acesso controlado a recursos de baixo nível como memória, arquivos e etc.
Essas chamadas seguem o seguinte fluxo:
- A CPU suspende a execução do programa.
- O controle é passado para o Kernel, que executa a solicitação da chamada.
- Após o serviço ser concluído, o controle é retornado para o programa, que continua sua execução.

As chamadas que você deve conhecer para este projeto são o `open()` e o `read()`. Mais tarde falaremos sobre eles. 

Agora que você já sabe o que são as Syscall, vamos voltar para o File Descriptor: um número inteiro será gerado pela chamada e será a nossa porta de conexão com o arquivo que iremos ler. Por padrão, o sistema operacional mantém uma tabela de descritores para cada processo em execução, então quando nosso FD recebe o inteiro 3, por exemplo, significa que o arquivo aberto ocupa a quarta posição dessa tabela interna no Kernel. É importante que você saiba também que, geralmente, essa tabela inicia no índice 3, pois todo programa em C já começa com 3 descritores ocupados por padrão, sendo eles: entrada de dados `stdin` no índice 0, saída de dados `stdout` no índice 1 e `stderr` que representa erro no índice 2. A quarta posição, ou o índice 3, é o menor índice disponível dessa tabela.
Além de ser um índice, o FD também consegue rastrear o progresso da sua leitura, funciona como um marcador de páginas: vamos imaginar que nosso arquivo .txt é um livro e você gostaria de lê-lo. Você pede para o Kernel o livro "arquivo.txt" e pede para que ele leia 10 páginas desse livro para você, o Kernel te entrega uma ficha numérica (o índice do arquivo aberto) e começa a ler o total e páginas que você solicitou. Após finalizar, ele coloca um marcador na página 11 para, quando você quiser ler o livro novamente, saber exatamente a página que parou e não precisar ler tudo do zero de novo, desde que você tenha a "ficha numérica" deste livro hipotético. Espero que essa explicação tenha ficado claro pois estamos lidando com baixo nível da programação aqui, então realmente é bastante complexo.

### open() e read()

Como você já sabe, o `open()` e o `read()` são chamadas de sistema (Syscall), mas o que eles fazem realmente? Vamos entender isso agora.

* **open():** É a porta de entrada. Ele não lê o conteúdo do arquivo, mas solicita o acesso ao Kernel. Você deve passar o caminho do arquivo (path) e qual a sua intenção (flag), por exemplo: `open("arquivo.txt", O_RDONLY)`, você está pedindo ao Kernel para verificar se o arquivo existe e se você possui permissão para lê-lo (*O_RDONLY: read only, ou traduzindo, somente leitura*), se a resposta for positiva, o Kernel retorna o número inteiro que será o seu File Descriptor, que explicamos anteriormente. Se o arquivo não existir ou se você não tiver permissão para lê-lo, então o Kernel retorna -1, indicando um erro. Existem outras flags além do `O_RDONLY`, mas não irei explicar aqui para não desviar muito do nosso objetivo, porém vale a pena pesquisar quais são as essas flags e como elas funcionam caso tenha despertado sua curiosidade.

  Exemplo de como usar o open: `int fd = open("text.txt", O_RDONLY);`

 * **read():** O `read()` é quem de fato faz o transporte de dados do disco para a memória do seu programa. Ele recebe o seu **FD**, estabelecendo conexão com o arquivo e sabendo exatamente a partir de onde ele deve ler, o **buffer**, que é o espaço de memória reservado por você através do `malloc()` para guardar esses dados e o **tamanho** que você quer ler deste arquivo, esse tamanho nós iremos chamar de `BUFFER_SIZE`. O Kernel então consulta o "marcador da página" associado ao nosso FD, transfere a quantidade de dados compatível com o nosso BUFFER_SIZE e os guarda no nosso espaço de memória reservado. Após isso ele atualiza o "marcador da página" (chamamos esse marcador de *offset*) para o estado atual, para que na próxima chamada o sistema saiba exatamente onde continuar a transferência dos dados. O read() retorna a quantidade de bytes lidos, ao encerrar os dados ele retornará 0 e em caso de erros retornará -1.

	Exemplo de como usar o read: `ssize_t readed = read(fd, buffer, BUFFER_SIZE);`

### Gerenciamento de Memória (malloc e free)

Antes de falarmos sobre variáveis estáticas, precisamos garantir que o conceito de gerenciamento de memória está sólido em sua "memória" (hehe), pois esse é um dos grandes desafios na linguagem C. Caso você nunca tenha ouvido falar sobre Alocação Dinâmica de Memória, faremos um breve resumo agora, para que todos estejam na mesma página.
A Alocação Dinâmica permite que um programa consiga reservar um espaço na memória durante a sua execução, em vez de reservá-la fixamente no código fonte durante a compilação, o que chamamos Alocação Estática. A memória dinâmica permite que o programador tenha controle total sobre o tempo de vida e o tamanho daquela memória reservada. Mas isso também exige um grau de responsabilidade muito alto, pois toda memória alocada manualmente também deve ser devidamente liberada manualmente, para garantir que não haja nenhum vazamento de memória (memory leak). 

* **malloc():** Solicita ao sistema operacional um bloco específico de memória. Você deve informar o tamnho em bytes e a função retorna um ponteiro para o início desse bloco. Para descobrir o tamanho exato de bytes que você irá precisar, basta multiplicar o tamanho que você quer alocar * a quantidade de bytes que cada tipo carrega. Por exemplo, um char = 1 byte, neste caso se quiser alocar uma string com 100 caracteres nós chamamos o malloc(100 * sizeof(char)), reservando então um bloco de 100 bytes na memória. E se fosse um `int`? Bom, um inteiro possui 4 bytes, malloc(100 * sizeof(int)) resultaria em um bloco de memória de 400 bytes reservado.

	Exemplo de como usar o malloc: `char *buffer = (char *)malloc((BUFFER_SIZE + 1) * sizeof(char));`

	OBS.: No exemplo abaixo, usamos o BUFFER_SIZE que é uma macro definida no header, somamos ela com +1 pois criaremos uma string, isso significa que precisamos reservar 1 espaço para nulo ao final de toda string.


* **free():** O free é o comando que devolve a memória que você reservou de volta para o sistema. Em uma explicação mais superficial para não entrarmos em um submundo abstrato de mais, essa memória que foi alocada pelo `malloc` permanece ocupada até que o programa seja encerrado, mesmo que você pare de usá-la. Se você não deseja mais usar a memória alocada e esquecer de dar free nela, ela ficará presa até o programa ser finalizado, isso é o que chamamos de Memory Leak (vazamento de memória), que consome memória desnecessariamente. Em larga escala isso pode ser um problemão, então lembre-se, usou o malloc() e a variável não tem mais utilidade? `free(variável)`.

	Exemplo:
```
char *buffer = (char *)malloc((BUFFER_SIZE + 1) * sizeof(char));
	if (!buffer)
		return (NULL);
	line = read_file(fd, rest, buffer);
	free(buffer);
```
Neste caso a variável `buffer` foi já foi utilizada e nós queremos descartá-la, como ela foi alocada dinâmicamente, devemos usar o free para liberar essa memória (LEMBRE-SE: TODA MEMÓRIA ALOCADA DEVE SER LIBERADA!).

## Variáveis Estáticas (static)

Finalmente chegamos ao grande climax da get_next_line, as variáveis estáticas. 
Quando falamos de de variáveis estáticas, estamos falando de memória de longo prazo. Em uma função comum, quando você declara uma variável local, ela nasce no momento que a função começa e morre no momento em que a função termina. A função será iniciada do zero novamente, como se tivesse amnésia a cada chamada. Mas a palavra `static` muda tudo, quando você cria uma variável como static, essa memória é preservada mesmo após o termino da função, e ela ficará salva até o fim do programa. Guarde essa informação, ela será importante.
```
	static char	*rest; <<<<<<<<<<<
	char		*line;
	char		*buffer;
```

## Como de fato funciona a get_next_line

Agora que você já tem uma base sólida, posso explicar de fato como funciona a `get_next_line`. Pense nela como uma impressora de linhas que utiliza 2 ferramentas principais: o **FD** para encontrar o arquivo e o `read()` para transferir esses dados em pedaços definidos no seu **BUFFER_SIZE** e uma variável estática `static` para guardar a sobra após cadastring quebra de linha do pedaço transferido pelo `read()`.

### O Ciclo de funcionamento

O objetivo dessa função não é ler um arquivo inteiro, mas processá-lo em pequenos pedaços, encontrar uma quebra de linha (`\n`) ou um nulo (`\0`) e retornar uma string tratada, salvando a sobra em uma variável temporária. O fluxo segue a seguinte lógica:
1. Abrir o arquivo através do `open()` e guardar o **File Descripor**. Lembre-se que esse passo é responsabilidade da main e não se trata de uma funcionalidade da get_next_line, mas para que a função funcione sua main precisa fazer exatamente isso. Uma variável receberá uma string e, enquanto ela existir, nós chamamos o `get_next_line(fd)`, printamos a resposta na tela e depois damos **free** na string alocada, isso abre espaço para a próxima string que será retornada pela função até não haver string nenhuma.
```
int	main(void)
{
	int		fd;
	char	*line;

	fd = open("text.txt", O_RDONLY);
	line = 0;
	while ((line = get_next_line(fd)))
	{
		printf("%s", line);
		free(line);
	}
	close(fd);
	return (0);
}
```

2. A `get_next_line` receberá o **FD**. Criamos a variável estática para guardar o resto que sobrar depois de um `\n`, criamos uma outra variável para guardar o buffer (valor bruto) e uma última variável para guardar a linha depois de todos os tratamentos (valor lapdado). Como nós já temos o valor do BUFFER_SIZE (que é uma macro), nós já sabemos quanto espaço de memória iremos alocar, que será BUFFER_SIZE (+ 1 para o nulo) * sizeof de char. Esse espaço de memória alocado (buffer) receberá os dados transferidos pelo `read()` e, como a variável `buffer` não será mais usada, então damos **free** para liberá-la.
```
char	*get_next_line(int fd)
{
	static char	*rest;
	char		*line;
	char		*buffer;

	if (fd < 0 || BUFFER_SIZE < 1)
		return (NULL);
	buffer = (char *)malloc((BUFFER_SIZE + 1) * sizeof(char));
	if (!buffer)
		return (NULL);
	line = read_file(fd, rest, buffer);
	free(buffer);
	buffer = NULL;
	if (!line)
		return (NULL);
	rest = get_rest(line);
	line = first_line(line);
	return (line);
}
```

3. O `read()` retornará um produto bruto, devemos tratá-lo para que o resultado retornado seja uma string válida e concatenada com o resto que sobrou (caso tenha sobrado algum caracter após o `\n` anteriormente) e, caso seja a primeira volta, nós usamos a strdup para criar uma string vazia, pois a variável não carrega nenhum valor no seu início, e não podemos concatenar uma string com NULL.
O encerramento ocorre em três situações distintas: primeiro, se a função `read()` retornar -1, indicando um erro grave de sistema, o loop libera a memória de rest e aborta retornando NULL. Segundo, o loop utiliza um break quando a leitura chega ao fim do arquivo, o que é identificado quando a variável readed é igual a `'\0'` (ou zero), sinalizando que não há mais dados para processar. Por fim, existe uma condição de saída lógica através da função `ft_strchr(rest, '\n')`, que interrompe o ciclo assim que um caractere de nova linha é encontrado no rest, isso permite que o programa processe linha por linha sem precisar carregá-lo inteiramente.
```
static char	*read_file(int fd, char *rest, char *buffer)
{
	char	*temp;
	ssize_t	readed;

	while (1)
	{
		readed = read(fd, buffer, BUFFER_SIZE);
		if (readed == -1)
		{
			free(rest);
			return (NULL);
		}
		else if (readed == '\0')
			break ;
		buffer[readed] = '\0';
		if (rest == NULL)
			rest = ft_strdup("");
		temp = rest;
		rest = ft_strjoin(temp, buffer);
		free(temp);
		temp = NULL;
		if (ft_strchr(rest, '\n'))
			break ;
	}
	return (rest);
}
```
4. `get_rest` e `first_line` seguem a mesma lógica, mas com propósitos diferentes. A ideia aqui é utilizar a função substr para recortar toda a string até a quebra de linha, porém enquanto a função `first_line` retorna a primeira linha (incluindo a quebra de linha), a função `get_rest` retorna o resto após a quebra de linha, excluindo a mesma.

```
static char	*first_line(char *line)
{
	char	*line_cuted;
	int		position;

	position = (ft_strchr(line, '\n') - line) + 1;
	line_cuted = ft_substr(line, 0, position);
	free(line);
	return (line_cuted);
}

///////////////////////////////////////////////////////

static char	*get_rest(char *line)
{
	char	*rest;
	int		position;

	position = (ft_strchr(line, '\n') - line) + 1;
	rest = ft_substr(line, position, ft_strlen(line));
	if (*rest == '\0')
	{
		free(rest);
		rest = NULL;
		return (rest);
	}
	return (rest);
}
```
5. Após todo esse tratamento, atribuímos o valor de cada função acima para determinadas variáveis, sendo elas `static char *rest` que guardará o resto após o `\n` para a próxima rodada e `char *line` que será a linha tratada que retornaremos. A sobra da string após a quebra de linha já estará pronta para ser concatenada com o próximo resultado do `read()`.
```
rest = get_rest(line);
line = first_line(line);
```
6. repetimos esse processo até o fim do while na nossa main.

### O Gerenciamento de Memória

Como você não sabe o tamanho de cada linha ou quantas vezes o `read()` será chamado, utilizamos a memória dinâmica através do `malloc()` para criar o espaço necessário que guardará nossa string de retorno. O processo é simples, criamos uma variável temporária para guardar um valor **X** (que já está alocada) e concatenamos a string **temp** com a string **Y** ao mesmo tempo que atribuímos o valor do resultado para a string **X**, após isso, nós damos `free(temp)` para liberar a memória criada somente pra para auxiliar na concatenação de **X** com **Y**. A regra é clara, toda vez que você retornar uma variável nova criada a partir de uma variável alocada, deve-se dar **free** na variável anterior que já foi processada, caso contrário o programa sofrerá com Memory Leak, acumulando lixo de memória até o encerramento do programa.

# Referência

- `man 2 read` — manual oficial do Linux para a chamada de sistema read().
- `man 2 open` — manual oficial do Linux para a chamada de sistema open().
- Documentação de referência da linguagem C.
