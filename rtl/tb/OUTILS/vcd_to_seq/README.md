## VCD to SEQ
Traduction d'une trace VCD (=de simulation SystemVerilog) vers une trace SEQ manipulable dans le framework formel CADP.

Exécution : `make sim_seq`

Visualisation de trace dans bcg_edit : `make show_seq`

```
Graphe de dépendance des fichiers :

Scope.hpp ←   vcd_parser.hpp     
          ↙                   ↖
utils.hpp                       main.hpp
          ↖                   ↙
            seq_generator.hpp 
```

## 3) Vérification
Vvérification de conformité de la trace SEQ avec le modèle formel avec l'outil exhibitor.

Exécution : `make` ou `make default`
