>## 1 - Introdução:
>Essa documentação descreve a implementação e 
as instruções de uso do sistema de arquivos criado para distribuições GNU/Linux nos anos 90 **EXT2** (*second extended file system*).  
O **EXT2** é o sucessor do **EXT**, foi lançado em 1992 visando suprir algumas deficiências do sistema anterior. Dentre os aprimoramentos vale citar: a estrutura de [Inodes](link) torna-se mais robusta e flexível, a estrutura de diretórios passa a ser hierárquica ao invés de linear e o tamanho máximo de arquivos e partições é aumentado. Em resumo o *second extended file system* é um sistema de arquivos **simples, eficiente e leve**, atualmente é usado pedagogicamente e em sistemas legado.  
A presente implementação é uma verssão simplificada com **fins estritamente educacionais**. 

>### 1.1 - Contexto:
>Este projeto foi elaborado durante a disciplina de Sistemas Operacionais por alunos de graduação do curso de Bacharelado em Ciências da Computação da **Universidade Federal do Paraná** (Câmpus Campo Mourão) no primeiro semestre de 2025. 

>### 1.2 - Motivação:
>O tema deste projeto foi definido pelo professor de Sistemas Operacionais na etapa final do semestre quando o foco das aulas foi gerência de arquivos, tradicionalmente em todas as edições da disciplina é escolhido um sistema de arquivos que deve ser implementado por grupos de alunos.  
>A escolha foi feita considerando a relevância e o grau de complexidade do sistema de arquivos.

>### 1.3 - Objetivos:
>O cumprimento deste projeto visa contribuir na compreensão: de tópicos relacionados a gerência de arquivos em Sistemas Operacionais por meio de uma implementação simplificada de um sistema de arquivos clássico e do funcionamento das estruturas de dados em baixo nível encapsuladas pelas operações básicas em arquivos do **EXT2**.