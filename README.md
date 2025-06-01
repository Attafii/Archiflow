# Gestion des Projets - Documentation

## Description
Gestion des Projets est une application Qt C++ simple pour la gestion des projets d'architecture. Elle permet de suivre les projets, d'estimer leur durée et de gérer toutes les informations associées.

## Fonctionnalités
- **CRUD complet** : Ajout, modification, suppression et consultation des projets
- **Recherche et Tri** : Filtrage par nom, catégorie, dates et tri selon différents critères
- **Suivi du projet** : Suivi de la progression et des délais
- **Estimation automatique** : Calcul de la durée approximative du projet selon sa catégorie et sa complexité

## Structure du projet
- **src/** : Fichiers source C++
- **include/** : Fichiers d'en-tête
- **forms/** : Fichiers UI Qt Designer
- **resources/** : Ressources (styles)

## Modèle de données
Chaque projet contient les attributs suivants :
- ID projet
- Nom
- Catégorie
- Date de début
- Date de fin estimée
- Progression

## Interface utilisateur
L'application utilise un thème bleu moderne et une interface entièrement en français.

## Ouverture du projet
### Avec Qt Creator et CMake
1. Lancez Qt Creator
2. Sélectionnez "Fichier" > "Ouvrir fichier ou projet"
3. Naviguez jusqu'au dossier du projet et sélectionnez le fichier "CMakeLists.txt"
4. Cliquez sur "Ouvrir"
5. Configurez le projet pour votre kit de développement Qt 6

### Compilation et exécution
1. Configurez le projet pour votre kit de développement
2. Cliquez sur le bouton "Exécuter" (triangle vert) ou appuyez sur Ctrl+R

## Compilation avec CMake en ligne de commande
```
mkdir build
cd build
cmake ..
make
```

## Dépendances
- Qt 6.x
- Modules Qt requis : Core, Gui, Widgets, Network

## Remarques
- Cette application est une version simplifiée, conçue pour être facile à utiliser et à maintenir
- L'interface est intuitive et permet une gestion efficace des projets d'architecture
