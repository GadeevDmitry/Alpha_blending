# Alpha blending

## Введение

Alpha blending - это процесс объединения изображения (foreground) с фоном (background) для создания видимости частичной или полной прозрачности. Результатом объединения пикселя изображения и фона с компонентами $ (RGBA)_{fore} $ и $ (RGBA)_{back} $ соответственно является пиксель с компонентами $ (RGBA)_{blend} $, где каждый компонент $ X_{blend} $ пересчитывается по формуле:

$$
X_{blend} = X_{fore} * A_{fore} + X_{back} * (1 - A_{fore})
$$

Пример alpha-blending:

![Alpha blending](data/blend.bmp)

## Реализация

Для оптимизации пересчета цветов пикселей можно использовать SIMD инструкции. В данной работе представлено 2 версии пересчета с использованием векторизации и одна скалярная реализация.

Для поиска среднего времени выполнения цикла пересчета, он отдельной программой запускался `10000` для векторных реализаций и `5000` раз  для скалярной при размерах `1000*1000` изображения и фона.

Среда тестирования:

| CPU                   | Compiler   | OS                     |
|-----------------------|------------|------------------------|
| AMD Ryzen 7 PRO 5850U | GCC 11.3.0 | Linux Mint 21 Cinnamon |

Флаги сборки:

```
-O3 -mavx2
```

Результаты тестирования:

| Версия   | Время, мс | Коэффициент ускорения относительно предыдущей версии | Коэффициент ускорения относительно начальной версии |
|----------|-----------|------------------------------------------------------|-----------------------------------------------------|
| simple   | 10.284    | 1.00 (начальная версия)                              | 1.00 (начальная версия)
| intrin   |  1.338    | 7.69                                                 | 7.69
| improved |  1.237    | 1.08                                                 | 8.31

Описание версий (исходный код версий в src/alpha_blending.cpp):

`simple`   - попикселья обработка

`intrin`   - векторизация, вычисляющая по формуле $X_{blend} = X_{fore} * A_{fore} + X_{back} * (1 - A_{fore})$

`improved` - векторизация, вычисляющая по формуле $X_{blend} = (X_{fore} - X_{back}) * A_{fore} + X_{back}$

## Вывод

### [***intrin***](https://godbolt.org/z/v36boc7ov) & [***improved***](https://godbolt.org/z/5Wz75EGn1)

`intrin` и `improved` версии обрабатывают 4 пикселя за инструкцию. `improved` немного быстрее, так как исполбзует одну операцию умножения вместо двух в `intrin` версии.

![improved screen](godbolt_screen/improved.png "improved")
![intrin   screen](godbolt_screen/intrin.png   "intrin")

`simple`, помимо пересчета каждого пикселя по формуле выше, в добавок делает битовые сдвиги для извлечения и загрузки компонент в 32-битное целое и много операций обращения к памяти. Поэтому ускорение больше, чем в 4 раза.

## Как запустить тестирование

OS Linux:

```
cd src
make timer
./time                      // run all versions
./time --intrin --improved  // run only simd versions
./time --help               // show manual
```

Настройки (кол-во тестов для усреднения, размеры изображений и фона и т.д.) в src/settings.h
