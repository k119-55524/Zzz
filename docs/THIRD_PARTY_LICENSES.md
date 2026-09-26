# 📜 Сторонние библиотеки и лицензии (Third-Party Licenses)

Настоящий документ содержит реестр сторонних открытых библиотек и компонентов, используемых в движке ZzzEngine и инструментарии разработки, с указанием авторов, репозиториев и условий распространения.

В соответствии с требованиями лицензий MIT и Apache 2.0, данный реестр и тексты лицензий должны распространяться вместе с готовыми сборками (в виде файла `THIRD_PARTY_LICENSES.txt` в папке с игрой либо через окно About / Credits в графическом интерфейсе).

---

## 1. Реестр компонентов

| Компонент | Назначение в проекте | Автор / Правообладатель | Лицензия | Ссылка на репозиторий |
| :--- | :--- | :--- | :---: | :--- |
| **stb (stb_image, stb_image_resize2)** | Загрузка и ресайз исходных изображений (PNG, JPG, TGA, BMP, HDR) | Sean Barrett (nothings) | MIT / Public Domain | [github.com/nothings/stb](https://github.com/nothings/stb) |
| **DirectXTex** | Компрессор текстур в форматы BC1..BC7 и обработка мипмапов (Desktop) | Microsoft Corporation | MIT | [github.com/microsoft/DirectXTex](https://github.com/microsoft/DirectXTex) |
| **astc-encoder (astcenc)** | Компрессор текстур в формат ASTC (Mobile Android/iOS) | ARM Limited | Apache 2.0 | [github.com/ARM-software/astc-encoder](https://github.com/ARM-software/astc-encoder) |
| **basis_universal** | Универсальный кроссплатформенный компрессор текстур | Binomial LLC / Khronos Group | Apache 2.0 | [github.com/BinomialLLC/basis_universal](https://github.com/BinomialLLC/basis_universal) |
| **googletest** | Фреймворк модульного тестирования (Google Test) | Google Inc. | BSD-3-Clause | [github.com/google/googletest](https://github.com/google/googletest) |
| **benchmark** | Фреймворк производительности (Google Benchmark) | Google Inc. | Apache 2.0 | [github.com/google/benchmark](https://github.com/google/benchmark) |
| **nlohmann/json** | Парсинг и сериализация JSON | Niels Lohmann | MIT | [github.com/nlohmann/json](https://github.com/nlohmann/json) |
| **doxygen-awesome-css** | Оформление документации Doxygen | jothepro | MIT | [github.com/jothepro/doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) |

---

## 2. Тексты лицензий

### MIT License

```text
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

### Apache License, Version 2.0

```text
                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.
      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or Derivative
          Works a copy of this License; and
      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and
      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work; and
      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file.

   5. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE.
```
