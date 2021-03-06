// Copyright 2015-2017 Piperift. All Rights Reserved.

#include "JinkCorePrivatePCH.h"
#include "Entity.h"

#include "Item.h"
#include "Kismet/KismetMathLibrary.h"
#include "SummonList.h"


// Sets default values
AEntity::AEntity()
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CharacterMovement = GetCharacterMovement();

    MaxLive = 100;
    Live = MaxLive.BaseValue;
    Damage = 10;
    FireRate = 0.7f;
    BulletSpeed = 500;

    Faction = FFaction();

    WalkSpeed = 250;
    RunSpeed = 400;
    MovementState = EMovementState::MS_Walk;

    bIsSummoned = false;
}

void AEntity::OnConstruction(const FTransform & Transform) {
    //Update Live if its pure
    if(Live == MaxLive.BaseValue)
        Live = MaxLive;

    SetMovementState(MovementState);

    //Bind Movement change
    WalkSpeed.OnModified.BindDynamic(this, &AEntity::OnMovementAttributeModified);
    RunSpeed.OnModified.BindDynamic(this, &AEntity::OnMovementAttributeModified);
}

// Called when the game starts or when spawned
void AEntity::BeginPlay()
{
    Super::BeginPlay();

    //Create start buffs
    for(auto& Class : BuffsAtStart) {
        ApplyBuff(Class);
    }


    for (auto& ItemType : ItemsAtStart) {
        PickUpItem(ItemType);
    }

    OnTakeAnyDamage.AddDynamic(this, &AEntity::ReceiveDamage);
}

// Called every frame
void AEntity::Tick( float DeltaTime )
{
    Super::Tick( DeltaTime );

}



/**
 * Begin ATTRIBUTES
 */


/** End ATTRIBUTES*/

/**
* Begin ITEMS
*/
UItem* AEntity::PickUpItem(TSubclassOf<UItem> Type)
{
    if (!Type.Get()->IsChildOf<UItem>()) return 0;

    //Check if any buff restricts the pickup
    for (auto* Buff : Buffs) {
        if(!Buff->CanPickUpItem(Type)) {
            return nullptr;
        }
    }

    UItem** LastItemPtr = Items.FindByPredicate([Type](const auto* Item) {
        return Item->IsA(Type);
    });

    //Found another item of the same type.
    if (LastItemPtr && *LastItemPtr) {
        UItem* LastItem = *LastItemPtr;

        //Drop item if it's corrupted (not picked up)
        if (!LastItem->IsPickedUp()) {
            DropItem(LastItem);
        }
        else {
            if (LastItem->bUnique) {
                //It's unique. We dont want to pick up another.
                return nullptr;
            }
            else if (LastItem->bStackable) {
                //It's stackable. Pick it up on the same Item.
                LastItem->PickUp(this);
                return LastItem;
            }
        }
    }
    
    if (UItem* Item = NewObject<UItem>(this, Type.Get())) {
        Items.Add(Item);
        Item->PickUp(this);
        OnItemPickUp(Item);
        return Item;
    }
    return nullptr;
}

void AEntity::DropItem(UItem* Item)
{
    if (!Item)
        return;

    //Drop and destroy an item
    if (Items.Contains(Item)) {
        OnItemDrop(Item);
        Item->Drop();
        Items.Remove(Item);
    }
}

void AEntity::DropAllItems(TSubclassOf<UItem> Type)
{
    //Drop and destroy all items of a type
    Items.RemoveAll([Type](UItem* Item) {
        if (Item && Item->IsA(Type)) {
            Item->Drop();
            return true;
        }
        return false;
    });
}
void AEntity::ClearItems()
{
    for (auto* Item : Items) {
        if (Item) {
            Item->Drop();
        }
    }
    Items.Empty();
}
bool AEntity::HasItem(TSubclassOf<UItem> Type)
{
    if (!Type.Get()->IsChildOf<UItem>()) 
        return false;

    return Items.ContainsByPredicate([Type](auto* Item){
        return Item->IsA(Type);
    });
}
/** End ITEMS*/



bool AEntity::IsAlive() const
{
    return Live > 0;
}

bool AEntity::LiveIsUnderPercent(float Percent) const
{
    return Live/MaxLive < Percent/100;
}

FFaction AEntity::GetFaction() const
{
    return Faction;
}

void AEntity::SetFaction(const FFaction & InFaction)
{
    Faction = InFaction;
}

bool AEntity::IsHostileTo(const AActor* Other) {
    if (Other == nullptr) {
        UE_LOG(LogJinkCore, Warning, TEXT("JinkCore: AEntity::IsHostileTo tried to compare a Null Entity"));
        return false;
    }
    return IsHostileTowards(*Other);
}

bool AEntity::IsHostileToFaction(const FFaction Other) {
    return Faction.IsHostileTowards(Other);
}

void AEntity::SetMovementState(const EMovementState State)
{
    MovementState = State;

    if (CharacterMovement)
    {
        switch (MovementState)
        {
        case EMovementState::MS_None:
            CharacterMovement->MaxWalkSpeed = 0;
            break;
        case EMovementState::MS_Walk:
            CharacterMovement->MaxWalkSpeed = WalkSpeed;
            break;
        case EMovementState::MS_Run:
            CharacterMovement->MaxWalkSpeed = RunSpeed;
            break;
        default:
            //Don't do anything
            break;
        }
    }
}

void AEntity::RotateTowards(FRotator Rotation)
{
    CharacterMovement->bUseControllerDesiredRotation = true;
    if (Controller) {
        Controller->SetControlRotation(Rotation);
    }
}

void AEntity::RotateTowardsActor(AActor * Actor)
{
    if (Actor == nullptr)
        return;

    const FVector Direction = Actor->GetActorLocation() - this->GetActorLocation();
    RotateTowards(Direction.ToOrientationRotator());
}

void AEntity::Die(AController * InstigatedBy, AEntity * Killer)
{
    if (!IsAlive())
        return;

    Live = 0;

    JustDied_Internal(InstigatedBy, Killer);
}

ASpell* AEntity::CastSpell(TSubclassOf<ASpell> SpellType, AEntity* Target, FVector Location, FRotator Rotation, float _Damage)
{
    ASpell* Spell = Cast<ASpell>(GetWorld()->SpawnActor(SpellType, &Location, &Rotation));
    if (Spell) {
        Spell->Cast(this, Target, _Damage);
        return Spell;
    }
    return NULL;
}

ASpell * AEntity::CastSpellAtCaster(TSubclassOf<ASpell> SpellType, AEntity * Target, float _Damage)
{
    return CastSpell(SpellType, Target, this->GetActorLocation(), this->GetActorRotation(), _Damage);
}

void AEntity::ReceiveDamage_Implementation(AActor * DamagedActor, float _Damage, const class UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
    if (!IsAlive())
        return;

    if (AEntity* Causer = Cast<AEntity>(DamageCauser)) {
        if (!Causer->CheckApplyAnyDamage(DamagedActor, _Damage, DamageType, DamageCauser)) {
            return;
        }
    }

    if (!CheckReceiveDamage(DamagedActor, _Damage, DamageType, DamageCauser))
        return;

    Live = FMath::Clamp(Live - _Damage, 0.0f, (float)MaxLive);

    if (!IsAlive()) {
        AEntity* Killer = nullptr;
        if (InstigatedBy != nullptr) {
            //If the controller is valid the killer is the controlled entity
            Killer = Cast<AEntity>(InstigatedBy->GetPawn());
        }
        if(!Killer) {
            //If theres no killer, asume it is the damagecauser actor.
            Killer = Cast<AEntity>(DamageCauser);
        }

        JustDied_Internal(InstigatedBy, Killer);
    }
}

void AEntity::DoMeleAttack_Implementation(AEntity* Target)
{
    UE_LOG(LogJinkCore, Log, TEXT("JinkCore: %s attacked but default behaviour is been called."), *this->GetName());
}

bool AEntity::ApplyRadialDamage(float _Damage, const FVector & Origin, float DamageRadius, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor * DamageCauser, bool bDoFullDamage, ECollisionChannel DamagePreventionChannel)
{
    if (!DamageCauser)
        DamageCauser = this;

    const bool checkSuccess = CheckApplyRadialDamage(_Damage, Origin, DamageRadius, DamageTypeClass, IgnoreActors, DamageCauser, bDoFullDamage, DamagePreventionChannel);
    if (checkSuccess) {
        return UGameplayStatics::ApplyRadialDamage(this, _Damage, Origin, DamageRadius, DamageTypeClass, IgnoreActors, DamageCauser, GetController(), bDoFullDamage, DamagePreventionChannel);
    }
    return false;
}

bool AEntity::ApplyRadialDamageWithFalloff(float _Damage, float MinimumDamage, const FVector & Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor * DamageCauser, ECollisionChannel DamagePreventionChannel)
{
    if (!DamageCauser)
        DamageCauser = this;

    const bool checkSuccess = CheckApplyRadialDamageWithFalloff(_Damage, MinimumDamage, Origin, DamageInnerRadius, DamageOuterRadius, DamageFalloff, DamageTypeClass, IgnoreActors, DamageCauser, DamagePreventionChannel);
    if (checkSuccess) {
        return UGameplayStatics::ApplyRadialDamageWithFalloff(this, _Damage, MinimumDamage, Origin, DamageInnerRadius, DamageOuterRadius, DamageFalloff, DamageTypeClass, IgnoreActors, DamageCauser, GetController(), DamagePreventionChannel);
    }
    return false;
}

void AEntity::ApplyPointDamage(AActor * DamagedActor, float _Damage, const FVector & HitFromDirection, const FHitResult & HitInfo, TSubclassOf<class UDamageType> DamageTypeClass, AActor * DamageCauser)
{
    if (!DamagedActor)
        return;

    if (!DamageCauser)
        DamageCauser = this;

    const bool checkSuccess = CheckApplyPointDamage(DamagedActor, _Damage, HitFromDirection, HitInfo, DamageTypeClass, DamageCauser);
    if (checkSuccess) {
        UGameplayStatics::ApplyPointDamage(DamagedActor, _Damage, HitFromDirection, HitInfo, GetController(), DamageCauser, DamageTypeClass);
    }
}

void AEntity::ApplyDamage(AActor * DamagedActor, float _Damage, TSubclassOf<class UDamageType> DamageTypeClass, AActor * DamageCauser)
{
    if (!DamagedActor)
        return;

    if (!DamageCauser)
        DamageCauser = this;

    const bool checkSuccess = CheckApplyDamage(DamagedActor, _Damage, DamageTypeClass, DamageCauser);
    if (checkSuccess) {
        UGameplayStatics::ApplyDamage(DamagedActor, _Damage, GetController(), DamageCauser, DamageTypeClass);
    }
}

//APPLY CHECKS
bool AEntity::CheckApplyRadialDamage_Implementation(float _Damage, const FVector & Origin, float DamageRadius, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor * DamageCauser, bool bDoFullDamage, ECollisionChannel DamagePreventionChannel)
{ return true; }
bool AEntity::CheckApplyRadialDamageWithFalloff_Implementation(float _Damage, float MinimumDamage, const FVector & Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor * DamageCauser, ECollisionChannel DamagePreventionChannel)
{ return true; }
bool AEntity::CheckApplyPointDamage_Implementation(AActor * DamagedActor, float _Damage, const FVector & HitFromDirection, const FHitResult & HitInfo, TSubclassOf<class UDamageType> DamageTypeClass, AActor * DamageCauser)
{ return true; }
bool AEntity::CheckApplyDamage_Implementation(AActor * DamagedActor, float _Damage, TSubclassOf<class UDamageType> DamageTypeClass, AActor * DamageCauser)
{ return true; }
bool AEntity::CheckApplyAnyDamage_Implementation(AActor * DamagedActor, float _Damage, const class UDamageType * DamageType, AActor * DamageCauser)
{ return true; }

//RECEIVE CHECKS
bool AEntity::CheckReceiveDamage_Implementation(AActor * DamagedActor, float _Damage, const class UDamageType * DamageType, AActor * DamageCauser)
{ return true; }


void AEntity::JustDied_Internal(AController * InstigatedBy, AEntity * Killer)
{
    JustDied(InstigatedBy, Killer);
    JustDiedDelegate.Broadcast(InstigatedBy, Killer);
    if (IsPlayerControlled()) {
    }
    /*else if (ABasic_Con* AI = GetAI()) {
        AI->JustDied_Internal(InstigatedBy, Killer);
    }*/
}

UBuff * AEntity::ApplyBuff(TSubclassOf<UBuff> Class)
{
    if (!Class.Get()->IsChildOf<UBuff>()) return nullptr;

    UBuff* Buff = Cast<UBuff>(NewObject<UBuff>(this, Class));
    if (Buff) {
        Buff->Apply(this);
        Buffs.Add(Buff);
    }
    return Buff;
}

void AEntity::RemoveBuff(UBuff* Buff)
{
    if (!Buff) return;

    if(Buffs.Remove(Buff)) {
        Buff->Unapply();
    }
}

bool AEntity::HasBuff(UBuff* Buff)
{
    if (!Buff) return false;
    return Buffs.Contains(Buff);
}

bool AEntity::HasBuffOfClass(TSubclassOf<UBuff> Class)
{
    if (!Class.Get()->IsChildOf<UBuff>()) return false;

    return Buffs.ContainsByPredicate([Class](UBuff* Buff) {
        return Buff->IsA(Class.Get());
    });
}

const TArray<UBuff*>& AEntity::GetBuffs() {
    return Buffs;
}

/**
 * SUMMONING
 */
AEntity* AEntity::Summon(UClass* Class, FTransform Transform) {
    //Check that Class is a child of Entity
    if (!Class->IsChildOf(AEntity::StaticClass()))
        return nullptr;

    UWorld* World = GetWorld();
    if(World) {
        AEntity* SummonedEntity = CastChecked<AEntity>(World->SpawnActor(Class, &Transform));
        if(SummonedEntity) {
            SummonedEntity->SetupSummon(this);
        }
        return SummonedEntity;
    }

    return nullptr;
}

template<class T>
T AEntity::Summon(FTransform Transform) {
    return CastChecked<T>(Summon(T::StaticClass(), Transform));
}

void AEntity::SetupSummon(AEntity* InSummoner) {
    if (InSummoner) {
        SetOwner(InSummoner);
        bIsSummoned = true;
        Summoner = InSummoner;

        //Call Events
        JustSummoned(Summoner);
    }
    else {
        UE_LOG(LogJinkCore, Warning, TEXT("JinkCore: Tried to summon an entity of class '%s', but the summoner was null."), *StaticClass()->GetName());
    }
}

USummonList* AEntity::CreateSummonList() {
    USummonList* SummonList = NewObject<USummonList>(this);
    SummonList->Construct(this);
    return SummonList;
}
