/* 
 * This file is part of the UnixCommons distribution (https://github.com/yoori/unixcommons).
 * UnixCommons contains help classes and functions for Unix Server application writing
 *
 * Copyright (c) 2012 Yuri Kuznecov <yuri.kuznecov@gmail.com>.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



namespace
{
  std::string root_path;

  struct lang_case_test_data
  {
    const char sample[256];         // input data for ToLower and ToUpper
    const char sample_upper[256];
    const char sample_mix_lower_upper[256];
    const char ethalon_lower[256];  // standard for results
    const char ethalon_upper[256];
  } tds[4] =
  {
    // French
    {
      "De nombreux projets sont à l'étude pour produire de l'électricité à"
      " partir de la seule force des vagues.",
      "DE NOMBREUX PROJETS SONT À L'ÉTUDE POUR PRODUIRE DE L'ÉLECTRICITÉ À"
      " PARTIR DE LA SEULE FORCE DES VAGUES.",
      "DE NOMBREUX PROJETS SONT À L'ÉTUDE POUR PRODUIRE DE l'électricité à"
      " partir de la seule force des vagues.",
      // standard French results
      "de nombreux projets sont à l'étude pour produire de l'électricité à"
      " partir de la seule force des vagues.",
      "DE NOMBREUX PROJETS SONT À L'ÉTUDE POUR PRODUIRE DE L'ÉLECTRICITÉ À"
      " PARTIR DE LA SEULE FORCE DES VAGUES."
    },
    // Russian
    {
      "Строка на русском",
      "СТРОКА НА РУССКОМ",
      "СТРОКА НА русском",
      // standard Russian results
      "строка на русском",
      "СТРОКА НА РУССКОМ"
    },
    // Arabian texts
    {
      "واقفًا وإلى ورائه عامود على النحو المألوف في التماث",
      "واقفًا وإلى ورائه عامود على النحو المألوف في التماث",
      "واقفًا وإلى ورائه عامود على النحو المألوف في التماث",
      // standard Arabian results
      "واقفًا وإلى ورائه عامود على النحو المألوف في التماث",
      "واقفًا وإلى ورائه عامود على النحو المألوف في التماث"
    },
    // Deutsch texts
    {
      "Sie die einmalige Gebühr bezahlt haben können Sie dieses Bild in "
      "anderen Produkten oder weiteren Bestellungen ohne Gebühr verwenden."
      " Es können zusätzliche Kosten entstehen wenn Ihre Bestellung mehr "
      "als ein GRATIS Produkt enthält.",
      "SIE DIE EINMALIGE GEBÜHR BEZAHLT HABEN KÖNNEN SIE DIESES BILD IN "
      "ANDEREN PRODUKTEN ODER WEITEREN BESTELLUNGEN OHNE GEBÜHR VERWENDEN."
      " ES KÖNNEN ZUSÄTZLICHE KOSTEN ENTSTEHEN WENN IHRE BESTELLUNG MEHR "
      "ALS EIN GRATIS PRODUKT ENTHÄLT.",
      "SIE DIE EINMALIGE GEBÜHR BEZAHLT HABEN KÖNNEN SIE DIESES BILD IN "
      "ANDEREN PRODUKTEN ODER weiteren Bestellungen ohne Gebühr verwenden."
      " Es können zusätzliche Kosten entstehen wenn Ihre Bestellung mehr "
      "als ein GRATIS Produkt enthält.",
      // standard Deutsch results
      "sie die einmalige gebühr bezahlt haben können sie dieses bild in "
      "anderen produkten oder weiteren bestellungen ohne gebühr verwenden."
      " es können zusätzliche kosten entstehen wenn ihre bestellung mehr "
      "als ein gratis produkt enthält.",
      "SIE DIE EINMALIGE GEBÜHR BEZAHLT HABEN KÖNNEN SIE DIESES BILD IN "
      "ANDEREN PRODUKTEN ODER WEITEREN BESTELLUNGEN OHNE GEBÜHR VERWENDEN."
      " ES KÖNNEN ZUSÄTZLICHE KOSTEN ENTSTEHEN WENN IHRE BESTELLUNG MEHR "
      "ALS EIN GRATIS PRODUKT ENTHÄLT."
    },
  };

} // namespace

