/* Public fallback for a profile whose actual exploit remains imported locally. */
#include <stdio.h>
#include <unistd.h>

__attribute__((constructor)) static void local_payload_required(void) {
  dprintf(STDERR_FILENO,
          "[perfil-local] Este perfil exige o arquivo .so privado importado em Configuracoes > Payload local.\n"
          "[perfil-local] Importe o payload experimental fornecido separadamente e mantenha o modo Online.\n"
          "[perfil-local] Nenhum exploit foi executado por este arquivo de aviso.\n");
  _exit(43);
}
