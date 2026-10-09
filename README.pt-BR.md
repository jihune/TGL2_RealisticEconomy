# Realistic Economy, um mod para This Grand Life 2

Versão 1.0.0 · para a versão v1.03.22 do jogo · não oficial

[English](README.md) · [한국어](README.ko.md) · [Deutsch](README.de.md) · [Español](README.es.md) · [Français](README.fr.md) · [日本語](README.ja.md) · [简体中文](README.zh-CN.md)

O dinheiro em This Grand Life 2 passa a seguir regras mais próximas da vida real, e carregar um jogo salvo anterior deixa de dar vantagem. O mod não cria nenhuma janela nem botão novo: ele funciona dentro das telas que o jogo já tem, e seus textos aparecem no idioma do jogo (nos oito).

## Novidades

### Empréstimos e dinheiro
- Os juros de um empréstimo seguem a nota de crédito da sua família (de AAA a B) e, em uma hipoteca, a parte do preço que é financiada.
- O resumo mensal diz quanto dinheiro o próximo fim de mês vai levar.
- O resumo mensal avisa de um imóvel à venda bem abaixo do valor.

### Carregar um jogo salvo não dá lucro
- Se você carregar um jogo salvo de antes de um fim de mês que já passou, as negociações de ações e futuros ficam bloqueadas até esse fim de mês passar de novo.
- Os jogos do cassino têm aposta fixa e chances fixas, cada um uma vez por mês. O resultado continua valendo depois de carregar.
- Os candidatos de um mês são os mesmos depois de carregar.

### Mercado de ações
- Cada empresa mostra PER, PBR, rendimento do dividendo, o preço em relação ao mês anterior e um preço justo. Uma empresa perto da falência ou de emitir novas ações é sinalizada antes.
- Assinatura de pesquisa: Shift-clique em uma empresa e a pesquisa dela é refeita todo mês.
- Abrir o capital de uma empresa deixa 45% das ações com você e paga o restante em dinheiro. Cada 20% que você tem garante uma cadeira no Conselho de Administração.
- Mais ordenações para a lista de empresas e a lista de futuros, e uma visão de seis meses nos gráficos.

### Empresas
- A janela de contratação mostra quanto custa uma hora de trabalho (salário ÷ eficiência), do mais barato para o mais caro.
- Você pode entregar tarefas ao mod, empresa por empresa: funcionários (contratar quando falta gente, demitir quem está sobrando, responder a pedidos de aumento), ativos que faltam, anúncios, assinatura de contratos, mais espaço físico. Um Ctrl+Shift-clique entrega a empresa inteira.

### Falhas do próprio jogo, corrigidas
- O jogo fechava depois de carregar jogos salvos muitas vezes.
- Textos errados ou quebrados nas traduções do jogo (principalmente em coreano) e quadradinhos nos nomes das pessoas.
- Uma formação concluída (diploma, certificado) perdia valor todo mês.

## Instalação
1. Feche o jogo. Extraia o zip de Releases na pasta do jogo (a que contém `TGL2.exe`).
2. Dê um duplo clique em `RealisticEconomy_install.bat`. Antes de tudo ele copia seus jogos salvos para `saves_before_RealisticEconomy_1`.
3. Abra o jogo. A versão no canto inferior direito do menu principal mostra "v1.03.22 + Realistic Economy".

Para remover o mod, execute `RealisticEconomy_uninstall.bat`. Os jogos salvos com o mod abrem sem ele.

## Como usar
Quase tudo funciona sozinho. Os cliques são estes; o texto que aparece ao passar o mouse em cada lugar também explica.

| Onde | Clique | O que faz |
|---|---|---|
| Lista de empresas na janela do mercado de ações | Shift-clique em uma empresa | liga ou desliga a assinatura de pesquisa dessa empresa |
| A mesma lista | Ctrl+Shift-clique | liga ou desliga a assinatura de todas as empresas |
| Um ícone de anúncio na janela de uma empresa | Shift-clique | entrega os anúncios ao mod, ou pega de volta |
| O ícone Contratos na janela de uma empresa | Shift-clique | entrega a assinatura de contratos ao mod, ou pega de volta |
| Um ícone de anúncio na janela de uma empresa | Ctrl+Shift-clique | entrega a empresa inteira ao mod, ou pega de volta |
| O interruptor de gerenciamento automático na aba de funcionários | clique | muda para todos os funcionários dessa empresa |
| O interruptor "comprar de novo os ativos gastos" de uma empresa | clique | o mod compra também os ativos que faltam |

Cada função pode ser desligada em `RealisticEconomy.ini`. A descrição completa está em inglês dentro do zip: `RealisticEconomy_README_en.txt`.

## Bom saber
- Funciona apenas com a versão v1.03.22 do jogo. Em qualquer outra versão o mod se desliga sozinho.
- Um antivírus pode desconfiar de `version.dll`. É o Ultimate ASI Loader, público (MIT): o arquivo que carrega o mod quando o jogo abre.
- Sem relação com o desenvolvedor do jogo. Licença MIT; o código-fonte está em `src`.
