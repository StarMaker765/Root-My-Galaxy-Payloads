# SM-S928B / EZIF / 6.1.162 — KernelSU 3.3.1 r2 de diagnostico

O catalogo agora aponta para o payload r2 completo, disponivel para download e verificacao no Root My Galaxy Next. O proprietario relatou carregamento do KernelSU e estabilidade nessa execucao da r2. Nao houve teste independente no aparelho nem validacao prolongada.

**Esta variante isola os modulos durante o carregamento.** Ela nao valida o funcionamento completo dos modulos. A verificacao DEFEX do auxiliar permanece pendente: o log fornecido registrou retorno 42, embora o Next tenha confirmado o KernelSU por seus canais independentes. A adaptacao Samsung/DEFEX continua embutida; o retorno nao permite declarar que o bypass esteja confirmado no aparelho.

## Preparar o cache Offline

1. Atualize a fonte `StarMaker765/Root-My-Galaxy-Payloads`, branch `main`. Selecione o perfil cujo nome inclui `r2 DIAGNOSTIC - isolated modules`.
2. Remova a substituicao em **Local payload** para usar o arquivo publicado. Deixe o modo **Online**, Shizuku e **Install KernelSU** habilitados. Mantenha **Auto soft restart** e as automacoes de root desativadas durante este teste.
3. Se o KernelSU ja estiver carregado, faca um reinicio completo antes da nova execucao e inicie novamente o Shizuku. O payload recusa carregar por cima de um KernelSU ativo.
4. Execute uma vez em Online. Confirme tanto o carregamento quanto a mensagem de que o payload foi salvo em cache. Entao selecione Offline para as proximas execucoes.

A r2 importada anteriormente nao coincidia com o hash r1 declarado no catalogo, o que explicava a recusa do cache. O catalogo agora declara os bytes exatos da r2. Isso corrige a verificacao dos arquivos, mas a gravacao efetiva do cache ainda depende de uma execucao no telefone.

## Comportamento da r2

O exploit ZZI4 e o daemon Samsung KernelSU 3.3.1 sao identicos aos da r1. O auxiliar usa montagens privadas com diretorios vazios para ocultar, apenas do carregador, os modulos, as atualizacoes pendentes, o backup antigo e os scripts de inicializacao. A r2 nao renomeia ou exclui esses diretorios originais nem altera seus marcadores disable/remove. Outros processos continuam vendo os originais. Uma montagem recusada ou um alvo de metamodulo inesperado interrompe a tentativa antes do carregamento do KernelSU.

Os registros de etapas ficam em `/data/local/tmp/rmg332-diag-*/phase.log`, sem listar nomes de modulos. O leitor `r2/ler-etapas-rmg-r2.sh` pode reuni-los pelo mesmo terminal Shizuku usado no Next. Ele nao exige root no aplicativo do terminal.

O carregamento normal ainda pode instalar o daemon e seus recursos em `/data/adb/ksud` e `/data/adb/ksu`. O auxiliar preserva um daemon anterior no caminho `ksud-before` informado no log. Nao ha gravacao de particoes de firmware nem desativacao de watchdog nesta variante.

## Integridade e limites

- Payload r2: 5.719.704 bytes, SHA-256 `cb9920d28a81a2f0b8684d19f6070ad9a4e6b15398ea562cdb3984974cc3f758`.
- Daemon do perfil: 5.566.824 bytes, SHA-256 `3487bde89b33043bd6aa09db68d21a6375aaa126e131fda53b6f6318327b0081`.
- Alvo: SM-S928B ARM64, kernel EZIF/ZZI4 6.1.162, Shizuku/shell UID 2000 e KernelSU Manager oficial 3.3.1.
- O modulo embutido declara `6.1.166-dirty`. O relato positivo nao garante estabilidade em outros aparelhos, kernels ou configuracoes.
- A inicializacao do daemon se destaca em outro processo. O retorno do processo pai nao comprova que todas as etapas terminaram. A verificacao unica do auxiliar foi preservada da r1 nesta variante de diagnostico.
- O identificador historico do perfil termina em `local-r1` e foi mantido para preservar a selecao; o nome exibido e o hash identificam a r2 atual.

Os arquivos r1 e o antigo placeholder permanecem por historico, mas nao sao selecionados por esta entrada atual do catalogo. Os logs do telefone, o APK, a imagem boot e os relatorios locais nao fazem parte desta publicacao.

## Fontes

O codigo do wrapper e do auxiliar r2 esta em `r2/sources/`. A licenca original Apache-2.0 do auxiliar permanece em `sources/LICENSE-original-helper.txt`. O auxiliar deriva de [su_daemon.c, revisao 6e3223e6](https://github.com/BuSung-dev/Root-My-Galaxy-Payloads/blob/6e3223e689688540060ddd97a0927e927bfed207/src/su_daemon.c).

O daemon/modulo Samsung 3.3.1 e reutilizado sem alteracao de [Root-My-Galaxy-Payloads, revisao a2b22606](https://github.com/rushiranpise/Root-My-Galaxy-Payloads/tree/a2b22606e5e681f8649cc18010f06447787a79bf/kernelsu), que fornece as adaptacoes Samsung e os procedimentos de compilacao. Codigo base GPL: [KernelSU v3.3.1](https://github.com/tiann/KernelSU/tree/v3.3.1). Contexto da adaptacao DEFEX: [HMA-OSS #137](https://github.com/frknkrc44/HMA-OSS/issues/137).
