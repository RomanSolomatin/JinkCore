// Fill out your copyright notice in the Description page of Project Settings.

#include "JinkCorePrivatePCH.h"
#include "AttributesFunctionLibrary.h"

void UAttributesFunctionLibrary::SetOwner(UPARAM(ref) FEntityAttribute& Attribute, AEntity* _Owner) {
    Attribute.Owner = _Owner;
}

bool UAttributesFunctionLibrary::Equals(UPARAM(ref)FEntityAttribute & A, UPARAM(ref)FEntityAttribute & B)
{
    return A == B;
}

const float UAttributesFunctionLibrary::GetValue (const FEntityAttribute & Attribute)
{
    return Attribute;
}

const float UAttributesFunctionLibrary::ToFloat (const FEntityAttribute & Attribute)
{
    return GetValue(Attribute);
}

const float& UAttributesFunctionLibrary::GetBase (const FEntityAttribute & Attribute)
{
    return Attribute.BaseValue;
}

void UAttributesFunctionLibrary::AddModification(FEntityAttribute & Attribute, FAttributeModification & Modification)
{
    Attribute.AddModification(Modification);
}

/*
void UAttributesFunctionLibrary::AddLinkedModification(FEntityAttribute& Attribute, FAttributeModification& Modification, AActor* Owner) {
Modification.Owner = Owner;
Modification.bLinkedToOwner = true;
Attribute.AddModification(Modification);
}*/

void UAttributesFunctionLibrary::RemoveModification(FEntityAttribute & Attribute, FAttributeModification & Modification)
{
    Attribute.RemoveModification(Modification);
}
