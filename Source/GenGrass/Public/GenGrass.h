#pragma once
//-----------------HEADER----------
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

class GENGRASS_API FGenGrass: public IModuleInterface
{
public:
	static inline FGenGrass& Get()
	{
		return FModuleManager::LoadModuleChecked<FGenGrass>("GenGrass");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("GenGrass");
	}
public:

	/* This will get called when the editor loads the module */
	virtual void StartupModule() override;

	/* This will get called when the editor unloads the module */
	virtual void ShutdownModule() override;
};