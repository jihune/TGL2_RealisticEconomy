# Realistic Economy

Un mod para This Grand Life 2 que cambia las reglas del dinero del juego. Un hogar con muchas deudas paga más interés por sus préstamos. Mirar los precios de las acciones del mes siguiente y cargar después una partida anterior para comprar ya no sirve. Tampoco sirve cargar una partida después de perder en el casino. La ventana de Bolsa muestra las cifras que hacen falta para valorar una empresa. Cuando tienes varios negocios, puedes dejar al mod las contrataciones, los anuncios y los contratos. También se corrigen algunos fallos del propio juego.

Solo funciona con la versión v1.03.22 del juego. No lo ha hecho el desarrollador del juego. La versión actual es la 1.0.0.

Otros idiomas: [English](README.md), [한국어](README.ko.md), [Deutsch](README.de.md), [Français](README.fr.md), [Português (Brasil)](README.pt-BR.md), [日本語](README.ja.md), [简体中文](README.zh-CN.md)

## Qué cambia

### Interés de los préstamos
El interés de un préstamo pasa a ser el tipo de interés del Banco Central más un margen, y el margen depende de la calificación crediticia de tu hogar. Hay seis niveles, de AAA a B. La calificación sale de cuatro cosas: la deuda frente a los activos, los pagos de un año frente a los ingresos de un año, el efectivo frente a los gastos de un año y el patrimonio neto. Una hipoteca cuesta más cuanto mayor es la parte del precio de la casa que pides prestada. Las ventanas de préstamos muestran tu calificación junto al tipo de interés.

### El efectivo que se llevará el fin de mes
El resumen mensual, la ventana que se abre al cambiar de mes, empieza con la línea "Fin de mes: efectivo necesario $X". Es la suma de los pagos de tus préstamos y de lo que se llevaron de verdad los salarios, el alquiler y otros gastos en el último fin de mes.

### Bloqueo de operaciones después de cargar
Si cargas una partida de antes de un fin de mes que ya has jugado, no puedes comprar ni vender acciones ni futuros hasta que ese fin de mes pase otra vez. Así no se gana dinero con precios que ya has visto. El bloqueo dura dos meses como mucho. Una partida guardada después del fin de mes más lejano al que has llegado no se bloquea. Esta función se puede desactivar en la configuración.

### Casino
Los cuatro juegos pasan a tener una apuesta fija y probabilidades fijas.

| Juego | Apuesta | Premio |
|---|---|---|
| Tragaperras | $1,000 | 22 % de recuperar 3 veces la apuesta, 0,8 % de 40 veces |
| Ruleta | $10,000 | 44 % de 2 veces, 2 % de 4 veces |
| Blackjack | $100,000 | 45 % de 2 veces, 2 % de 2,5 veces |
| Bacará | $1,000,000 | 46,5 % de 2 veces |

Cada juego se puede jugar una vez al mes. El dinero se mueve al terminar la jugada. Guardar justo antes del final y cargar una y otra vez da siempre el mismo resultado. Si pierdes y luego cargas una partida anterior, la pérdida se descuenta otra vez de tu efectivo nada más cargar. Las apuestas suben con los precios del juego.

### Ventana de Bolsa
Con una empresa seleccionada, la primera pestaña muestra junto a los importes del juego el PBR, el PER, la rentabilidad por dividendo y el cambio del precio respecto al mes anterior. Al pasar el ratón por una empresa de la lista se ven las mismas cifras en una línea. Una empresa que has investigado en el juego muestra además su precio justo.

Una empresa que está a punto de quebrar o de emitir acciones nuevas lleva un aviso. Si tienes acciones de una empresa así, el resumen mensual también lo dice. El juego solo avisa cuando ya ha pasado.

Con Mayús+clic en una empresa de la lista, el mod repite su investigación cada mes y cobra una cuota cada vez. Ctrl+Mayús+clic lo hace para todas las empresas.

Cuatro de los cinco botones de orden que hay sobre la lista ordenan ahora por capitalización, por las más infravaloradas, por las más sobrevaloradas y por la subida del último mes. El gráfico de una empresa se abre solo con el precio de la acción, y los gráficos tienen una vista de seis meses. En la lista de futuros cada elemento muestra su tasa de inflación actual y la esperada.

### Salida a bolsa y Consejo de Administración
En el juego, sacar un negocio a bolsa te da el 25 % de las acciones y nada de efectivo. Con el mod te quedas el 45 %, y el otro 55 % se vende al precio de salida y se te paga en efectivo. Ese efectivo es ingreso sujeto a impuestos de ese año.

Cada 20 % que tengas de una empresa cotizada garantiza uno de los cinco puestos del Consejo de Administración. Aun así tienes que nominar a un miembro de tu hogar en el mes de la elección.

### Contratación y ofertas de contrato
La ventana de contratación muestra el salario por hora efectiva de cada candidato (salario por hora ÷ eficiencia laboral) y pone primero al más barato. Alguien que cobra $64.84 por hora con una eficiencia del 114 % cuesta $56.88. Los candidatos de un mes siguen siendo los mismos después de cargar una partida.

Pasa el ratón por el icono de pago de un contrato ofrecido para ver qué porcentaje paga por encima del coste estándar del trabajo. Cuando un negocio pierde eficiencia laboral porque le faltan activos, el resumen mensual lo nombra.

### Dejar un negocio al mod
En cada negocio puedes dejar al mod las tareas siguientes una por una. Contratar a alguien, mantener anuncios y firmar un contrato cuestan cada uno una cuota, que se calcula con el salario por hora de un empleo del juego.

- Empleados: activa el modo de gestión automática del juego, el icono de la flecha circular en la pestaña de empleados. El mod contrata a un candidato para un puesto al que le faltan manos. Cuando en un puesto sobran manos tres meses seguidos, despide a la persona más cara. Cuando un empleado pide un aumento, el mod lo sustituye por un candidato que hace el mismo trabajo por menos, y concede el aumento si no hay ninguno.
- Activos: marca en el negocio la casilla que compra activos nuevos automáticamente cuando caducan. Con el mod compra también los activos que faltan.
- Anuncios: Mayús+clic en un icono de anuncio. El mod activa los anuncios de pago mientras la conciencia está por debajo del 103 % y los desactiva por encima.
- Contratos: Mayús+clic en el icono de contratos. A principios de cada mes el mod firma las ofertas que pagan más que su coste estándar, tantas como admita el negocio. Solo lo hace en un negocio donde también le has dejado los empleados y los activos.
- El negocio entero: Ctrl+Mayús+clic en un icono de anuncio. Esto deja las cuatro tareas, y además el mod alquila más superficie cuando falta. Mientras el negocio está en manos del mod, los clics que lo cambian a mano quedan bloqueados (contratar, despedir, comprar y vender activos, aceptar y cancelar contratos). Otro Ctrl+Mayús+clic lo recupera.

Mientras un negocio está cerrado, el mod no hace nada en él hasta que lo abras otra vez.

### Propiedades en venta por debajo de su valor
Cuando una propiedad en venta tiene un precio muy por debajo de su valor, el resumen mensual da su dirección y lo que ganarías después de los gastos de compra.

### Educación terminada
El juego quita cada mes un 1 % del valor de un título o un certificado. A los cinco años queda cerca del 55 %, así que quien no empieza el trabajo poco después de titularse tiene que estudiar lo mismo otra vez. Con el mod, una educación que ha terminado un miembro del hogar no baja de lo que dio. La experiencia ganada trabajando sigue bajando como antes.

### Fallos del juego corregidos
- El juego se cerraba después de cargar partidas unas decenas de veces sin reiniciarlo.
- Se corrigen textos equivocados de las traducciones del juego. En español, el interés fijo aparecía como "Corregido" y ahora dice "Fijo", y un número de meses salía como "MESm".
- En algunos nombres de personas salían recuadros en lugar de letras.

## Antes de instalar
- Cuando una actualización cambia la versión del juego, el mod se desactiva solo. No cambia nada hasta que salga una edición para la versión nueva.
- Un antivirus puede desconfiar de `version.dll`. Es el Ultimate ASI Loader, de código público, el archivo que carga el mod al abrir el juego.
- La primera vez que cargas una partida de antes del mod, el modo de gestión automática de los empleados y la compra automática de activos se desactivan en todos los negocios, porque con el mod estos dos interruptores hacen más. Actívalos de nuevo en los negocios que quieras dejar al mod.
- El mod no añade ninguna ventana ni ningún botón. Sus textos salen dentro de las ventanas del juego, en el idioma del juego.

## Instalación
1. Cierra el juego.
2. Descarga el zip de [Releases](../../releases) y extráelo en la carpeta del juego, la que contiene `TGL2.exe`.
3. Haz doble clic en `RealisticEconomy_install.bat`. Antes de nada copia tus partidas a la carpeta `saves_before_RealisticEconomy_1`.
4. Abre el juego. El mod está instalado si detrás de la versión, en la esquina inferior derecha del menú principal, pone "+ Realistic Economy".

Para quitar el mod, cierra el juego y ejecuta `RealisticEconomy_uninstall.bat`. Las partidas guardadas con el mod se abren sin él.

## Configuración y descripción completa
Cada función se puede desactivar en `RealisticEconomy.ini`, que queda junto a `TGL2.exe` después de instalar. El manual con las cifras y las cuotas de cada regla está en inglés: [docs/RealisticEconomy_README_en.txt](docs/RealisticEconomy_README_en.txt). También viene dentro del zip.

## Licencia y código fuente
Licencia MIT. El código fuente está en la carpeta [src](src).
