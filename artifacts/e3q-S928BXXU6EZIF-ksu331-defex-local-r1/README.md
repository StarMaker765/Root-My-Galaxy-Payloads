# SM-S928B / EZIF / 6.1.162 — perfil experimental com payload local

Este perfil destina-se a um teste particular. A entrada no catálogo não representa confirmação de funcionamento em hardware.

O exploit real deve ser importado em Settings > Payload Management > Local payload. Ele não é publicado neste diretório. O arquivo local-payload-required.so apenas mostra uma mensagem e encerra com código 43 se o arquivo privado não tiver sido importado. Ele não explora o kernel e não concede root.

Use o modo Online para o primeiro teste; o modo Offline depende de um cache anterior verificado. Habilite Shizuku e Install KernelSU. O payload particular fornecido separadamente exige SM-S928B, ARM64 e kernel EZIF/ZZI4 6.1.162. Use o KernelSU Manager oficial 3.3.1.

O daemon disponibilizado é uma cópia sem alterações do ksud-generic-kdp 3.3.1 público do repositório rushiranpise/Root-My-Galaxy-Payloads, revisão a2b22606e5e681f8649cc18010f06447787a79bf. SHA-256: 3487bde89b33043bd6aa09db68d21a6375aaa126e131fda53b6f6318327b0081.

Seu módulo para android14-6.1 declara 6.1.166-dirty. Compatibilidade com o kernel EZIF 6.1.162 não está confirmada. Listar 6.1.162 neste catálogo identifica o aparelho pretendido para teste; não altera o módulo nem torna essa compatibilidade validada.

A adaptação Samsung contém o bypass DEFEX broad-root (tarefas UID 0 fora do domínio KernelSU), conforme a discussão HMA-OSS #137. A integração particular verifica o parâmetro após carregar o módulo. Funcionalidade no system_server/HMA depende de teste e do reinício userspace necessário ao root pós-boot.

Fontes do daemon/módulo GPL: https://github.com/tiann/KernelSU/tree/v3.3.1 e https://github.com/rushiranpise/Root-My-Galaxy-Payloads/tree/a2b22606e5e681f8649cc18010f06447787a79bf/kernelsu . As alterações Samsung e os procedimentos de compilação estão nesse último repositório.
