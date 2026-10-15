# SM-S928B / EZIF / 6.1.162 — KernelSU 3.3.1 + DEFEX r1

Este perfil agora disponibiliza o payload completo para download e cache no Root My Galaxy Next. O proprietário do aparelho relatou funcionamento em 15/10/2026; não houve teste independente em hardware. O arquivo é exatamente o mesmo r1 usado no teste local, sem alterações no exploit ou no auxiliar.

## Usar o cache Offline

1. Remova a substituição em Settings > Payload Management > Local payload. Não é mais necessário importar um `.so`.
2. Atualize a fonte `StarMaker765/Root-My-Galaxy-Payloads`, branch `main`. Se houver uma revisão antiga fixada, libere essa fixação. Selecione `Galaxy S24 Ultra EZIF | 6.1.162 | KSU 3.3.1 + DEFEX | EXPERIMENTAL r1`.
3. Para uma nova execução, desative o root automático e reinicie normalmente para descarregar o KernelSU anterior. Inicie o Shizuku, mantenha Install KernelSU habilitado e faça uma instalação manual no modo Online.
4. Após a instalação bem-sucedida, confira se o Next informa que há um payload em cache. Então selecione o modo Offline. A publicação no GitHub permite a verificação dos arquivos; a criação efetiva do cache depende dessa execução no telefone.

O cache do Next inclui o perfil, o exploit e o daemon verificados, e registra o hash do auxiliar que acompanha o aplicativo. Uma atualização desse auxiliar pode exigir nova execução Online. Reinstalar/limpar os dados do aplicativo também remove o cache.

## Arquivos e integridade

- Payload completo: `payload-S928B-6.1.162-KSU331-DEFEX-r1.so`, 5.715.496 bytes, SHA-256 `99d3a7bd9f79cba40cf0030127c004ca98b9736e2a453fcab6a7ca7799afaea1`.
- Daemon do perfil: `kernelsu/ksud-e3q-S928BXXU6EZIF-3.3.1-generic-kdp`, 5.566.824 bytes, SHA-256 `3487bde89b33043bd6aa09db68d21a6375aaa126e131fda53b6f6318327b0081`.
- O antigo `local-payload-required.so` permanece apenas por histórico; este perfil não o utiliza mais.

O identificador `e3q-S928BXXU6EZIF-ksu331-defex-local-r1` foi mantido para preservar a identidade do perfil. O sufixo histórico `local` não exige mais um arquivo local.

## Escopo

Destinado ao SM-S928B ARM64 com kernel 6.1.162 EZIF/ZZI4, via Shizuku/shell UID 2000 e KernelSU Manager oficial 3.3.1. O auxiliar verifica a versão de kernel do KernelSU 32680 e `samsung_defex_broad_root=Y`. Não grava partições de firmware. O carregamento pode atualizar `/data/adb/ksud`; um daemon anterior é preservado no caminho `ksud-before` exibido no log.

O módulo Samsung android14-6.1 embutido no daemon declara `6.1.166-dirty`. O relato positivo é específico ao aparelho/firmware acima, não uma garantia para outros kernels. O bypass DEFEX broad-root cobre tarefas UID 0 fora do domínio KernelSU; não cria uma exceção indiscriminada para processos UID 1000. O funcionamento de cada módulo, incluindo HMA/system_server, depende de teste separado.

## Procedência e fontes

- Exploit ZZI4 original, sem alterações: SHA-256 `4f4255f51618e0a4044f383234f72822128e1efbca78e41149416afb52b73566`.
- Auxiliar baseado em [su_daemon.c, revisão 6e3223e6](https://github.com/BuSung-dev/Root-My-Galaxy-Payloads/blob/6e3223e689688540060ddd97a0927e927bfed207/src/su_daemon.c). Fontes do wrapper e do auxiliar adaptado, com a licença original do auxiliar, estão em `sources/`.
- Daemon/módulo público Samsung 3.3.1 de [rushiranpise/Root-My-Galaxy-Payloads, revisão a2b22606](https://github.com/rushiranpise/Root-My-Galaxy-Payloads/tree/a2b22606e5e681f8649cc18010f06447787a79bf/kernelsu). Esse diretório contém as adaptações Samsung e os procedimentos de compilação. Código base GPL: [KernelSU v3.3.1](https://github.com/tiann/KernelSU/tree/v3.3.1).
- Contexto DEFEX: [HMA-OSS #137](https://github.com/frknkrc44/HMA-OSS/issues/137).

O APK, a imagem boot e os relatórios locais não fazem parte desta publicação.
