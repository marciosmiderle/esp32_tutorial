# OTA versão 3

## Implementação
### Criar a biblioteca libota [3 dias]
### Incluir no projeto Envio_e_Comunicacao a libota [2 dias]
### Refatorar do projeto Envio_e_Comunicacao o que é OTA para a libota : classes acessórias estarão dentro da libota [7 dias]
### Incluir no projeto RINC a libota [20 dias]

## Testes são criados paralelamente ao desenvolvimento


```mermaid

gantt
    title OTA versão 3
    dateFormat YYYY-MM-DD
    axisFormat %d/%m
    excludes weekends

    section Implementação
    Criar a biblioteca libota :a1, 2026-11-13, 3d
    Incluir no projeto Envio_e_Comunicacao a libota :a2, after a1, 2d
    Refatorar o que é OTA de Envio_e_Comunicacao para libota (classes acessórias) :a3, after a2, 7d
    Incluir no projeto RINC a libota :a4, after a3, 20d

    section Qualidade
    Testes (criados paralelamente) :active, t1, 2026-11-13, 32d

```
