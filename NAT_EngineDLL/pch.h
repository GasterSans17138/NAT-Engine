// pch.h : Il s'agit d'un fichier d'en-tête précompilé.
// Les fichiers listés ci-dessous sont compilés une seule fois, ce qui améliore les performances de génération des futures builds.
// Cela affecte également les performances d'IntelliSense, notamment la complétion du code et de nombreuses fonctionnalités de navigation du code.
// Toutefois, les fichiers listés ici sont TOUS recompilés si l'un d'entre eux est mis à jour entre les builds.
// N'ajoutez pas de fichiers fréquemment mis à jour ici, car cela annule les gains de performance.

#ifndef PCH_H
#define PCH_H

#ifdef NAT_EngineDLL
	#define NAT_API __declspec(dllexport)
#else
	#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

// ajouter les en-têtes à précompiler ici
#include "framework.h"

//#define 

//#include "Jolt/Jolt.h"
//#include "assimp/Importer.hpp"
//#include "GLFW/glfw3.h"
//#include "vulkan/vulkan.h"
//#include "ImGUI/imgui.h"

#endif //PCH_H
