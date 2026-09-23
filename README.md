# AIS_2627
AIS Group Assigments

## Trabalho 1 — Exercício 2: utilizador com a shell `bach`

O objetivo é criar o utilizador Linux `iselx`, definir o programa `bach` como a sua shell de login e testar o seu funcionamento nessa conta. Este exercício é demonstrado durante a discussão.

### 1. Abrir o Ubuntu no WSL

No PowerShell ou no terminal do Windows:

```powershell
wsl -d Ubuntu
```

Os comandos de preparação seguintes são executados no Bash do Ubuntu, com o utilizador habitual que tem acesso a `sudo`.

### 2. Compilar e instalar a shell

Se o GCC ainda não estiver instalado:

```bash
sudo apt update
sudo apt install build-essential
```

Entrar na pasta do código. Este é o caminho usado neste ambiente; adaptar se o repositório estiver noutro local:

```bash
cd /mnt/c/Users/gonca/Documents/Licenciatura/ano_3/AIS/repo/AIS_2627/src/Assigment1
```

Compilar e instalar, copiando a linha completa:

```bash
gcc -Wall -Wextra -std=gnu11 bach.c -o /tmp/bach-cw1 && sudo install -m 755 /tmp/bach-cw1 /usr/local/bin/bach
```

O `&&` só permite a instalação se a compilação tiver sucesso. O executável fica em `/usr/local/bin/bach`, com permissões de execução para o novo utilizador. O binário é gerado fora do repositório e não deve ser incluído no Git.

Confirmar a instalação:

```bash
ls -l /usr/local/bin/bach
```

As permissões esperadas são `-rwxr-xr-x`.

### 3. Criar o utilizador e definir a palavra-passe

Executar este passo apenas na primeira configuração. Se a conta já tiver sido criada seguindo este guia, avançar para a verificação com `getent`.

```bash
sudo useradd -m -s /usr/local/bin/bach iselx
sudo passwd iselx
```

- `-m` cria a pasta pessoal, normalmente `/home/iselx`.
- `-s` define o caminho absoluto do executável que será usado como shell de login.
- `passwd` pede a nova palavra-passe duas vezes; os caracteres não aparecem enquanto se escreve.

Verificar a configuração:

```bash
getent passwd iselx
```

A linha deverá terminar em:

```text
:/home/iselx:/usr/local/bin/bach
```

### 4. Entrar na conta e testar os comandos básicos

A partir da conta habitual, iniciar a shell de login de `iselx`:

```bash
su --login iselx
```

Introduzir a palavra-passe de `iselx`. No prompt do `bach`, executar uma linha de cada vez:

```text
whoami
pwd
printenv SHELL
ls
echo teste com argumentos
cd /tmp
pwd
cd /home/iselx
pwd
sleep 2
exit
```

Resultados esperados:

- `whoami` mostra `iselx`.
- O primeiro `pwd` mostra `/home/iselx`.
- `printenv SHELL` mostra a shell configurada: `/usr/local/bin/bach`.
- `ls` lista os ficheiros visíveis; pode não apresentar saída numa pasta sem ficheiros visíveis.
- `echo teste com argumentos` mostra `teste com argumentos`.
- Os dois `pwd` seguintes mostram `/tmp` e `/home/iselx`, respetivamente. É necessário um espaço em `cd /tmp`.
- `sleep 2` só devolve o prompt após terminar, cerca de dois segundos depois.
- `exit` termina o `bach` e regressa à sessão do utilizador anterior.

### 5. Completar os testes de pipes e redirecionamento

Estes testes ficam pendentes até as funcionalidades correspondentes do exercício 1 estarem concluídas. Na versão atual de `bach.c`, o bloco dos pipes está vazio e o redirecionamento ainda não separa corretamente o comando, os argumentos e o nome do ficheiro.

Depois de atualizar o código, repetir a compilação e instalação do passo 2. Não é necessário recriar o utilizador. Entrar novamente com `su --login iselx` e executar, uma linha de cada vez:

```text
cp /etc/passwd text.txt
cat text.txt
cat text.txt > copy.txt
diff text.txt copy.txt
cat text.txt | grep iselx
cat text.txt | grep iselx > output.txt
cat output.txt
exit
```

Resultados esperados: `diff` não apresenta diferenças entre `text.txt` e `copy.txt`; o pipe mostra a linha do utilizador `iselx`; `cat output.txt` mostra essa mesma linha. Os ficheiros de teste ficam na pasta pessoal de `iselx`.
