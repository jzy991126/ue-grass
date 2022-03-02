// Fill out your copyright notice in the Description page of Project Settings.

#include "GenGrass.h"

void FGenGrass::StartupModule()
{
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders/Private"));
	AddShaderSourceDirectoryMapping("/CustomShaders", ShaderDirectory);
}

void FGenGrass::ShutdownModule()
{

}


IMPLEMENT_GAME_MODULE(FGenGrass, GenGrass);

