# Realistic Economy

Realistic Economy é um mod para This Grand Life 2. O mod muda as regras de dinheiro do jogo. Ele impede o lucro que você pode conseguir quando carrega um jogo salvo. Ele adiciona dados à janela do mercado de ações e funções automáticas à janela da empresa. Ele corrige algumas falhas do jogo.

- Versão do mod: 1.0.0
- Versão do jogo: somente v1.03.22
- O desenvolvedor do jogo não fez este mod.

Outros idiomas: [English](README.md), [한국어](README.ko.md), [Deutsch](README.de.md), [Español](README.es.md), [Français](README.fr.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## O que o mod faz

### Juros dos empréstimos
A taxa de juros de um empréstimo é a taxa de juros do Banco Central mais uma margem. A nota de crédito da sua família define a margem. Há seis notas de crédito: AAA, AA, A, BBB, BB e B.

O mod calcula a nota de crédito com estes quatro valores:

- a dívida comparada com os ativos
- os pagamentos de empréstimos de um ano comparados com a renda de um ano
- o dinheiro comparado com os gastos de um ano
- o patrimônio líquido

Em uma hipoteca, a margem é maior quando você financia uma parte maior do preço da casa. A janela de empréstimo mostra a nota de crédito.

### Dinheiro para o fim do mês
O resumo mensal é a janela que abre quando o mês muda. O mod coloca a linha "Fim do mês: dinheiro necessário $X" no topo dessa janela. O valor é a soma das parcelas dos seus empréstimos e dos outros custos que você pagou no último fim de mês. Salários e aluguel são custos desse tipo.

### Bloqueio de negociação depois de carregar
Se você carrega um jogo salvo anterior a um fim de mês que já passou, o mod bloqueia as negociações. Durante o bloqueio, você não pode comprar nem vender ações. Você também não pode negociar futuros. O bloqueio termina quando você passa de novo por esse fim de mês. O bloqueio não dura mais de dois meses.

Sem o bloqueio, você pode ver os preços das ações do mês seguinte, carregar um jogo salvo antigo e comprar ações. Você pode desligar essa função no arquivo de configurações.

### Cassino
O mod dá aos quatro jogos uma aposta fixa e chances fixas.

| Jogo | Aposta | Resultado |
|---|---|---|
| Caça-níqueis | $1,000 | 22%: você recebe 3 vezes a aposta. 0,8%: você recebe 40 vezes a aposta. |
| Roleta | $10,000 | 44%: 2 vezes. 2%: 4 vezes. |
| Blackjack | $100,000 | 45%: 2 vezes. 2%: 2,5 vezes. |
| Bacará | $1,000,000 | 46,5%: 2 vezes. |

- Você pode jogar cada jogo uma vez por mês.
- Você recebe ou perde o dinheiro quando a ação do cassino termina.
- Se você salva e carrega de novo, o resultado é o mesmo.
- Se você perde e depois carrega um jogo salvo antigo, o mod tira de novo o dinheiro perdido.
- As apostas sobem com os preços do jogo.

### Janela do mercado de ações
O mod mostra estes valores para cada empresa:

- PBR
- PER
- rendimento do dividendo
- variação do preço da ação desde o mês passado
- preço justo (somente para uma empresa que você pesquisou no jogo)

Selecione uma empresa para ver os valores na primeira aba. Você também pode colocar o ponteiro do mouse sobre uma empresa da lista.

O mod mostra um aviso em uma empresa que está perto da falência, de uma redução de tamanho ou de uma emissão de novas ações. Se você tem ações dessa empresa, o resumo mensal também mostra o aviso.

Assinatura de pesquisa:

- Faça Shift-clique em uma empresa da lista. O mod pesquisa essa empresa de novo todo mês. Você paga uma taxa por cada pesquisa.
- Para assinar todas as empresas, faça Ctrl+Shift-clique em uma empresa.

Outras mudanças:

- Quatro dos cinco botões de ordenação têm uma ordem nova:
  - maior valor de mercado
  - menor preço comparado com o preço justo
  - maior preço comparado com o preço justo
  - maior alta desde o mês passado
- O gráfico de uma empresa abre somente com a linha do preço da ação.
- Os gráficos têm uma visão de 6 meses.
- A lista de futuros mostra a taxa de inflação e a taxa de inflação esperada de cada item.

### Abertura de capital e cadeiras no Conselho
No jogo, a abertura de capital da sua empresa dá a você 25% das ações. Você não recebe dinheiro. Com o mod, você fica com 45% das ações. O mod vende os outros 55% ao preço de abertura e dá o dinheiro a você. Esse dinheiro é renda tributável daquele ano.

Cada 20% das ações de uma empresa listada garante uma cadeira no Conselho de Administração. O Conselho tem cinco cadeiras. Indique um membro da sua família no mês da eleição. Se você não indica um membro, não recebe uma cadeira.

### Candidatos e ofertas de contrato
A janela de contratação mostra o pagamento por hora efetiva de cada candidato. Pagamento por hora efetiva = pagamento por hora ÷ eficiência no trabalho. Exemplo: $64.84 ÷ 114% = $56.88. A lista mostra primeiro o candidato com o menor valor.

Os candidatos de um mês não mudam quando você carrega um jogo salvo.

Coloque o ponteiro do mouse sobre o ícone de pagamento de uma oferta de contrato. O mod mostra quantos por cento a oferta paga acima do custo padrão do trabalho.

Se uma empresa não tem ativos suficientes, a eficiência no trabalho dela cai. O resumo mensal mostra então o nome dessa empresa.

### Automação de uma empresa
O mod pode fazer cinco tarefas para uma empresa. Você inicia cada tarefa em cada empresa.

| Tarefa | Como iniciar | O que o mod faz |
|---|---|---|
| Funcionários | Ligue o modo de gerenciamento automático do jogo. O ícone dele é a seta circular na aba de funcionários. | Contrata um candidato quando uma função não tem funcionários suficientes. Demite o funcionário mais caro quando uma função tem funcionários demais por três meses. Quando um funcionário pede aumento, troca o funcionário por um candidato mais barato. Se não há um candidato assim, aceita o pedido. |
| Ativos | Marque na empresa a caixa que compra novos ativos automaticamente. | Compra também os ativos que faltam. |
| Anúncios | Faça Shift-clique em um ícone de anúncio. | Liga os anúncios pagos quando a conscientização é menor que 103%. Desliga os anúncios quando a conscientização é de 103% ou mais. |
| Contratos | Faça Shift-clique no ícone de contratos. | No começo de cada mês, assina as ofertas que pagam mais do que o custo padrão. Assina tantas ofertas quantas a empresa comporta. Só faz isso quando as tarefas Funcionários e Ativos também estão ligadas. |
| Tudo | Faça Ctrl+Shift-clique em um ícone de anúncio. | Faz as quatro tarefas acima. Aluga mais espaço físico quando o espaço físico não é suficiente. Trava os cliques que mudam a empresa à mão. |

- Para parar uma tarefa, faça o mesmo clique de novo.
- O mod cobra uma taxa quando contrata um funcionário, mantém anúncios ligados ou assina um contrato.
- O mod não faz nada em uma empresa fechada.

### Imóveis abaixo do valor
Se um imóvel à venda tem um preço muito menor que o valor dele, o resumo mensal mostra uma linha. A linha dá o endereço e o ganho. O ganho é o valor menos o preço e as taxas de compra.

### Formação concluída
O jogo baixa todo mês em 1% o valor de um diploma ou de um certificado. Depois de cinco anos, sobram aproximadamente 55%. Com o mod, a formação concluída de um membro da sua família mantém o valor. Só a parte acima do requisito mais alto de um emprego baixa. A experiência do trabalho baixa como no jogo.

### Falhas do jogo corrigidas
- O jogo fechava quando você carregava jogos salvos algumas dezenas de vezes sem reiniciar. O mod corrige essa falha.
- Alguns textos traduzidos estavam errados. O mod corrige esses textos. Exemplo: em alguns textos aparecia uma palavra no lugar de um número.
- Algumas letras dos nomes de pessoas apareciam como quadradinhos. O mod mostra as letras corretas.

## Antes da instalação
- Se uma atualização muda a versão do jogo, o mod se desliga sozinho. Espere uma versão nova do mod.
- Um antivírus pode alertar sobre `version.dll`. Esse arquivo é o Ultimate ASI Loader, que é público. Ele carrega o mod quando o jogo inicia.
- Quando você carrega pela primeira vez um jogo salvo anterior ao mod, o mod desliga dois interruptores em todas as empresas. São o modo de gerenciamento automático dos funcionários e a compra automática de ativos. Com o mod, esses dois interruptores fazem mais tarefas. Ligue os dois de novo somente nas empresas que o mod deve cuidar.
- O mod não adiciona janelas nem botões. Os textos do mod aparecem nas janelas do jogo, no idioma do jogo.

## Instalação
1. Feche o jogo.
2. Baixe o arquivo zip em [Releases](../../releases).
3. Extraia o arquivo zip na pasta do jogo. A pasta do jogo é a pasta que contém `TGL2.exe`.
4. Dê um duplo clique em `RealisticEconomy_install.bat`. Esse arquivo copia primeiro seus jogos salvos para a pasta `saves_before_RealisticEconomy_1`.
5. Inicie o jogo.
6. Olhe o texto de versão no canto inferior direito do menu principal. Se o texto contém "+ Realistic Economy", o mod está instalado.

## Remoção
1. Feche o jogo.
2. Dê um duplo clique em `RealisticEconomy_uninstall.bat`.

Você pode abrir sem o mod os jogos que salvou com o mod.

## Configurações e manual
- O arquivo de configurações é `RealisticEconomy.ini`. Depois da instalação, ele fica na pasta do jogo. Nesse arquivo, você pode desligar cada função.
- O manual completo está em inglês: [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). Ele dá todos os números e todas as taxas das regras. O arquivo zip também contém o manual.

## Licença e código-fonte
A licença é MIT. O código-fonte está na pasta [src](src).
