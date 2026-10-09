# Realistic Economy, un mod para This Grand Life 2

Versión 1.0.0 · para la versión v1.03.22 del juego · no oficial

[English](README.md) · [한국어](README.ko.md) · [Deutsch](README.de.md) · [Français](README.fr.md) · [Português (Brasil)](README.pt-BR.md) · [日本語](README.ja.md) · [简体中文](README.zh-CN.md)

El dinero de This Grand Life 2 sigue reglas más cercanas a la vida real, y cargar una partida anterior ya no da ventaja. El mod no crea ninguna ventana ni ningún botón nuevo: trabaja dentro de las pantallas que el juego ya tiene, y sus textos salen en el idioma del juego (en los ocho).

## Novedades

### Préstamos y efectivo
- El interés de un préstamo sigue la calificación crediticia de tu hogar (de AAA a B) y, en una hipoteca, la parte del precio que pides prestada.
- El resumen mensual dice cuánto efectivo se llevará el próximo fin de mes.
- El resumen mensual avisa de una propiedad en venta muy por debajo de su valor.

### Cargar una partida no da ganancia
- Si cargas una partida de antes de un fin de mes que ya pasaste, las operaciones con acciones y futuros quedan bloqueadas hasta que ese fin de mes pase otra vez.
- Los juegos del casino tienen una apuesta fija y probabilidades fijas, cada uno una vez al mes. El resultado se mantiene después de cargar.
- Los candidatos de un mes son los mismos después de cargar.

### Bolsa
- Cada empresa muestra PER, PBR, rentabilidad por dividendo, el precio frente al mes anterior y un precio justo. Una empresa cerca de la quiebra o de emitir acciones nuevas queda señalada de antemano.
- Suscripción de investigación: haz Mayús+clic en una empresa y su investigación se renueva cada mes.
- Sacar una empresa a bolsa te deja el 45 % de las acciones y te paga el resto en efectivo. Cada 20 % que tengas garantiza un puesto en el Consejo de Administración.
- Más formas de ordenar la lista de empresas y la de futuros, y una vista de seis meses en los gráficos.

### Negocios
- La ventana de contratación muestra lo que cuesta una hora de trabajo (salario ÷ eficiencia), primero lo más barato.
- Puedes dejar tareas al mod, negocio por negocio: empleados (contratar cuando faltan manos, despedir a quien sobra, responder a las peticiones de aumento), activos que faltan, anuncios, firma de contratos, más superficie. Un Ctrl+Mayús+clic deja el negocio entero.

### Fallos del propio juego, corregidos
- El juego se cerraba después de cargar partidas muchas veces.
- Textos equivocados o rotos en las traducciones del juego (sobre todo en coreano) y recuadros en los nombres de las personas.
- Una educación terminada (título, certificado) perdía valor cada mes.

## Instalación
1. Cierra el juego. Extrae el zip de Releases en la carpeta del juego (la que contiene `TGL2.exe`).
2. Haz doble clic en `RealisticEconomy_install.bat`. Antes de nada copia tus partidas a `saves_before_RealisticEconomy_1`.
3. Abre el juego. La versión de la esquina inferior derecha del menú principal dice "v1.03.22 + Realistic Economy".

Para quitar el mod ejecuta `RealisticEconomy_uninstall.bat`. Las partidas guardadas con el mod se abren sin él.

## Cómo se usa
Casi todo funciona solo. Estos son los clics; el texto que aparece al pasar el ratón por cada sitio también los explica.

| Dónde | Clic | Qué hace |
|---|---|---|
| Lista de empresas de la ventana de Bolsa | Mayús+clic en una empresa | activa o desactiva la suscripción de esa empresa |
| La misma lista | Ctrl+Mayús+clic | activa o desactiva la suscripción de todas las empresas |
| Un icono de anuncio en la ventana de un negocio | Mayús+clic | deja los anuncios al mod, o los recupera |
| El icono Contratos en la ventana de un negocio | Mayús+clic | deja la firma de contratos al mod, o la recupera |
| Un icono de anuncio en la ventana de un negocio | Ctrl+Mayús+clic | deja el negocio entero al mod, o lo recupera |
| El interruptor de gestión automática en la pestaña de empleados | clic | lo cambia para todos los empleados de ese negocio |
| El interruptor «comprar de nuevo los activos gastados» de un negocio | clic | el mod compra también los activos que faltan |

Cada función se puede desactivar en `RealisticEconomy.ini`. La descripción completa está en inglés dentro del zip: `RealisticEconomy_README_en.txt`.

## Conviene saber
- Solo funciona con la versión v1.03.22 del juego. En cualquier otra versión el mod se desactiva solo.
- Un antivirus puede desconfiar de `version.dll`. Es el Ultimate ASI Loader, público (MIT): el archivo que carga el mod al abrir el juego.
- Sin relación con el desarrollador del juego. Licencia MIT; el código fuente está en `src`.
