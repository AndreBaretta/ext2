<p align="center">
<img src="Img/gnu.png" width="100">
<img src="Img/C.png" width="100" style="margin: 0px 150px 0px">
<img src="Img/tux.png" width="100">
</p>



---
# Implementação do sistema de arquivos EXT2

## 📝 Descrição:
- Este projeto é uma implementação do sistema de arquivos **EXT2** escrito em **linguagem C**, desenvolvido por alunos do Bacharelado de Ciências da Computação da **UTFPR** (Universidade Tecnológica Federal do Paraná - Câmpus Campo Mourão) para a discliplina de Sistemas Operacionais com **fins exclusivamente educacionais**.

## 🔨 Estruturas implementadas:
- Superbloco
- Descritor de bloco
- Inode
- i_osd2

## 🔧 Estrutura do projeto:
```bash
.                       #Dir. raíz
├── Img                 #Dir. com as Imagens do README.md
├── include             #Dir. de cabeçalhos do EXT2
│   ├── EXT2.h          #Estruturas
│   └── EXT2_Utils.h    #Assinaturas das funções
├── main.c              #Função Principal
├── Makefile            #Script para compilar o projeto
├── README.md           #Apresentação e instruções de uso
├── Shell               #Dir. do Terminal
│   ├── Shell.c         #Funções do terminal
│   └── Shell.h         #Cabeçalhos do terminal
└── src                 #Dir. de funções do EXT2
    └── EXT2_Utils.c    #implementação das funções
```
## ⚙️ Como compilar e executar:

### 📋 Pré-requisitos:
- Sistema operacional **GNU/Linux**
- `gcc` (compilador C)
- `make` (utilitário de build)

### ⚒️ Compilação:
- No diretório raiz do projeto, execute:

```bash
$ make
```

- Um binário executável e arquivos de objeto (.o) serão gerados após compilar o programa.
- Para usar o programa, execute:  

```bash
$ ./ext2shell <imagem.img>
```
- Uma Interface de linha de comando iniciará no seu terminal. Você podera manipular sua imagem **EXT2** usando ela. A seguir, apresentamos a documnetação dos comandos:  
  
  | **Comando**        | **Parâmetros**    | **Descrição**     
  |:---:               |:---    |:---
  | **`help`**         | **nenhum**    | exibe os comandos e instruções de uso. 
   `info`              | **nenhum**    | exibe informações do disco e do sistema de arquivos.
   `cat`               |`<arquivo>`    | exibe o conteúdo do arquivo fornecido no formato texto.
   `attr`              | `<arquivo_ou_diretório>`    | exibe os atributos do arquivo ou diretório fornecido.
   `print inode`       | `<numero_do_inode>`    | exibe informações do inode fornecido
   `print superblock`  |**nenhum**    | exibe informações do superbloco
   `cd`                |`<caminho>`    | altera o diretório corrente para o caminho fornecido.
   `ls`                |`<caminho>`    | lista os arquivos e sub-diretórios do diretório corrente.
   `pwd`               |**nenhum**    | exibe o diretório corrente (caminho absoluto).
   `touch`             |`<arquivo>`    | cria um arquivo com conteúdo vazio.
   `mkdir`             |`<diretório>`    | cria um diretório vazio.
    `rm `              |`<arquivo>`    | remove o arquivo do sistema.
    `rmdir`            |`<diretório>`    | remove o diretório fornecido, se estiver vazio.
    `rename`           |`<arquivo> <novo_nome_do_arquivo> `    | renomeia o arquivo para novo_nome_do_arquivo.
    `cp`               |`<caminho_EXT2> <caminho_máquina_real>` | copia o conteúdo de um arquivo da imagem EXT2 para um arquivo no disco rígido da máquina 
    `exit`             |**nehum**    | fecha a Interface de linha de comando.


- Os arquivos gerados pelo programa podem ser apagados de forma prática executando o comando `make clean`.  

## 📜 Licença

Este projeto está licenciado sob a Licença Pública Geral **GNU v3** — veja o arquivo `LICENSE` para detalhes.

---
## 👥 Autores

<b>
André Felipe Baretta<br>  
Guilherme Inoe Araujo<br>
Henry Meneguini Farias<br>
João Pedro Inoe Araujo
</b>
