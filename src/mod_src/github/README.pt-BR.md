# Realistic Economy

Um mod para This Grand Life 2 que muda as regras de dinheiro do jogo. Uma família com muitas dívidas paga mais juros nos empréstimos. Ver os preços das ações do mês seguinte e depois carregar um jogo salvo anterior para comprar não adianta mais. Carregar um jogo salvo depois de perder no cassino também não. A janela do mercado de ações mostra os números necessários para avaliar uma empresa. Quando você tem várias empresas, pode deixar as contratações, os anúncios e os contratos com o mod. Algumas falhas do próprio jogo também são corrigidas.

Funciona apenas com a versão v1.03.22 do jogo. Não foi feito pelo desenvolvedor do jogo. A versão atual é a 1.0.0.

Outros idiomas: [English](README.md), [한국어](README.ko.md), [Deutsch](README.de.md), [Español](README.es.md), [Français](README.fr.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## O que muda

### Juros dos empréstimos
Os juros de um empréstimo passam a ser a taxa de juros do Banco Central mais uma margem, e a margem depende da nota de crédito da sua família. São seis notas, de AAA a B. A nota sai de quatro coisas: a dívida em relação aos ativos, os pagamentos de um ano em relação à renda de um ano, o dinheiro em relação aos gastos de um ano e o patrimônio líquido. Uma hipoteca fica mais cara quanto maior for a parte do preço da casa que você financia. As janelas de empréstimo mostram a sua nota ao lado da taxa de juros.

### O dinheiro que o fim do mês vai levar
O resumo mensal, a janela que abre quando o mês muda, começa com a linha "Fim do mês: dinheiro necessário $X". É a soma das parcelas dos seus empréstimos com o que salários, aluguel e outros custos realmente levaram no último fim de mês.

### Bloqueio de negociação depois de carregar
Se você carregar um jogo salvo de antes de um fim de mês que já jogou, não pode comprar nem vender ações ou futuros até esse fim de mês passar de novo. Assim não se ganha dinheiro com preços já vistos. O bloqueio dura no máximo dois meses. Um jogo salvo depois do fim de mês mais distante a que você chegou não é bloqueado. Essa função pode ser desligada nas configurações.

### Cassino
Os quatro jogos passam a ter aposta fixa e chances fixas.

| Jogo | Aposta | Prêmio |
|---|---|---|
| Caça-níqueis | $1,000 | 22% de chance de receber 3 vezes a aposta, 0,8% de 40 vezes |
| Roleta | $10,000 | 44% de 2 vezes, 2% de 4 vezes |
| Blackjack | $100,000 | 45% de 2 vezes, 2% de 2,5 vezes |
| Bacará | $1,000,000 | 46,5% de 2 vezes |

Cada jogo pode ser jogado uma vez por mês. O dinheiro se move quando a rodada termina. Salvar logo antes do fim e carregar várias vezes dá sempre o mesmo resultado. Se você perder e depois carregar um jogo salvo anterior, a perda é descontada de novo do seu dinheiro logo após o carregamento. As apostas sobem com os preços do jogo.

### Janela do mercado de ações
Com uma empresa selecionada, a primeira aba mostra, ao lado dos valores do jogo, PBR, PER, rendimento do dividendo e a variação do preço em relação ao mês anterior. Ao passar o mouse sobre uma empresa da lista, você vê os mesmos números em uma linha. Uma empresa que você pesquisou no jogo mostra também o preço justo.

Uma empresa prestes a falir ou a emitir novas ações leva um aviso. Se você tem ações de uma empresa assim, o resumo mensal também avisa. O jogo só avisa depois que acontece.

Com Shift-clique em uma empresa da lista, o mod refaz a pesquisa dela todo mês e cobra uma taxa a cada vez. Ctrl+Shift-clique faz isso para todas as empresas.

Quatro dos cinco botões de ordenação acima da lista agora ordenam por valor de mercado, pelas mais subvalorizadas, pelas mais supervalorizadas e pela alta no último mês. O gráfico de uma empresa abre só com o preço da ação, e os gráficos ganham uma visão de seis meses. Na lista de futuros, cada item mostra a taxa de inflação atual e a esperada.

### Abertura de capital e Conselho de Administração
No jogo, abrir o capital de uma empresa dá a você 25% das ações e nenhum dinheiro. Com o mod você fica com 45%, e os outros 55% são vendidos ao preço de abertura e pagos a você em dinheiro. Esse dinheiro é renda tributável daquele ano.

Cada 20% que você tem de uma empresa listada garante uma das cinco cadeiras do Conselho de Administração. Mesmo assim você precisa indicar um membro da família no mês da eleição.

### Contratação e ofertas de contrato
A janela de contratação mostra o salário por hora efetiva de cada candidato (salário por hora ÷ eficiência no trabalho) e lista o mais barato primeiro. Quem recebe $64.84 por hora com 114% de eficiência custa $56.88. Os candidatos de um mês continuam os mesmos depois de carregar um jogo salvo.

Passe o mouse sobre o ícone de pagamento de um contrato oferecido para ver quantos por cento ele paga acima do custo padrão do trabalho. Quando uma empresa perde eficiência no trabalho porque faltam ativos, o resumo mensal diz qual é.

### Deixar uma empresa com o mod
Em cada empresa você pode entregar as tarefas abaixo uma a uma. Contratar alguém, manter anúncios e assinar um contrato custam cada um uma taxa, calculada pelo salário por hora de um emprego do jogo.

- Funcionários: ligue o modo de gerenciamento automático do jogo, o ícone de seta circular na aba de funcionários. O mod contrata um candidato para uma função em que falta gente. Quando sobra gente em uma função por três meses seguidos, ele demite a pessoa mais cara. Quando um funcionário pede aumento, o mod o troca por um candidato que faz o mesmo trabalho por menos, e concede o aumento se não houver nenhum.
- Ativos: marque na empresa a caixa que compra novos ativos automaticamente quando eles expiram. Com o mod ela compra também os ativos que faltam.
- Anúncios: Shift-clique em um ícone de anúncio. O mod liga os anúncios pagos enquanto a conscientização está abaixo de 103% e desliga acima disso.
- Contratos: Shift-clique no ícone de contratos. No começo de cada mês o mod assina as ofertas que pagam mais do que o custo padrão, tantas quantas a empresa comporta. Ele só faz isso em uma empresa em que os funcionários e os ativos também estão com ele.
- A empresa inteira: Ctrl+Shift-clique em um ícone de anúncio. Isso entrega as quatro tarefas, e o mod ainda aluga mais espaço físico quando falta. Enquanto a empresa está com o mod, os cliques que a alteram à mão ficam travados (contratar, demitir, comprar e vender ativos, aceitar e cancelar contratos). Outro Ctrl+Shift-clique pega a empresa de volta.

Enquanto uma empresa está fechada, o mod não faz nada nela até você abrir de novo.

### Imóveis à venda abaixo do valor
Quando um imóvel à venda está com preço bem abaixo do valor, o resumo mensal dá o endereço e quanto você ganharia depois das taxas de compra.

### Formação concluída
O jogo tira todo mês 1% do valor de um diploma ou certificado. Depois de cinco anos sobram cerca de 55%, então quem não começa o emprego logo depois do diploma precisa estudar a mesma coisa de novo. Com o mod, uma formação que um membro da família concluiu não cai abaixo do que ela deu. A experiência ganha trabalhando continua diminuindo como antes.

### Falhas do jogo corrigidas
- O jogo fechava depois de carregar jogos salvos algumas dezenas de vezes sem reiniciar.
- Textos errados nas traduções do jogo são corrigidos. Em vários idiomas, o português entre eles, aparecia uma palavra no lugar onde deveria estar um número ou uma data.
- Em alguns nomes de pessoas apareciam quadradinhos no lugar das letras.

## Antes de instalar
- Quando uma atualização muda a versão do jogo, o mod se desliga sozinho. Ele não muda nada até sair uma edição para a versão nova.
- Um antivírus pode desconfiar de `version.dll`. É o Ultimate ASI Loader, um carregador público, o arquivo que carrega o mod quando o jogo abre.
- Na primeira vez que você carrega um jogo salvo de antes do mod, o modo de gerenciamento automático dos funcionários e a compra automática de ativos são desligados em todas as empresas, porque com o mod esses dois interruptores fazem mais. Ligue de novo nas empresas que você quer deixar com o mod.
- O mod não cria nenhuma janela nem botão. Os textos dele aparecem dentro das janelas do jogo, no idioma do jogo.

## Instalação
1. Feche o jogo.
2. Baixe o zip em [Releases](../../releases) e extraia na pasta do jogo, a que contém `TGL2.exe`.
3. Dê um duplo clique em `RealisticEconomy_install.bat`. Antes de tudo ele copia seus jogos salvos para a pasta `saves_before_RealisticEconomy_1`.
4. Abra o jogo. O mod está instalado se depois da versão, no canto inferior direito do menu principal, aparecer "+ Realistic Economy".

Para remover o mod, feche o jogo e execute `RealisticEconomy_uninstall.bat`. Os jogos salvos com o mod abrem sem ele.

## Configurações e descrição completa
Cada função pode ser desligada em `RealisticEconomy.ini`, que fica ao lado de `TGL2.exe` depois da instalação. O manual com os números e as taxas de cada regra está em inglês: [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). Ele também vem dentro do zip.

## Licença e código-fonte
Licença MIT. O código-fonte está na pasta [src](src).
