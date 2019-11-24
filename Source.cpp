#include "../SDK/PluginSDK.h"
#include "../SDK/EventArgs.h"
#include "../SDK/EventHandler.h"

PLUGIN_API const char PLUGIN_PRINT_NAME[32] = "Teemo";
PLUGIN_API const char PLUGIN_PRINT_AUTHOR[32] = "Biggie";
PLUGIN_API ChampionId PLUGIN_TARGET_CHAMP = ChampionId::Teemo;

namespace Menu
{
	IMenu* MenuInstance = nullptr;

	namespace Combo
	{
		IMenuElement* Enabled = nullptr;
		IMenuElement* UseQ = nullptr;
		IMenuElement* UseQaa = nullptr;
		IMenuElement* UseR = nullptr;
	}

	namespace Harass
	{
		IMenuElement* Enabled = nullptr;
		IMenuElement* UseQ = nullptr;
		IMenuElement* UseQaa = nullptr;
		IMenuElement* TowerQ = nullptr;
		IMenuElement* ManaQ = nullptr;
	}

	namespace Killsteal
	{
		IMenuElement* Enabled = nullptr;
		IMenuElement* UseQ = nullptr;
	}

	namespace Misc
	{
		IMenuElement* UseWflee = nullptr;
		IMenuElement* UseWslow = nullptr;
		IMenuElement* Qdanger = nullptr;
		IMenuElement* Qdash = nullptr;
		IMenuElement* QantiMelee = nullptr;
	}
	namespace Drawings
	{
		IMenuElement* Toggle = nullptr;
		IMenuElement* DrawQRange = nullptr;
		IMenuElement* DrawRRange = nullptr;
	}

	namespace Colors
	{
		IMenuElement* QColor = nullptr;
		IMenuElement* RColor = nullptr;
	}
}

namespace Spells
{
	std::shared_ptr<ISpell> Q = nullptr;
	std::shared_ptr<ISpell> W = nullptr;
	std::shared_ptr<ISpell> E = nullptr;
	std::shared_ptr<ISpell> R = nullptr;
}

// combo
void ComboLogic()
{
	if (Menu::Combo::Enabled->GetBool() && Spells::Q->IsReady())
	{
		const auto OrbwalkerTarget = g_Orbwalker->GetTarget();
		if (OrbwalkerTarget && OrbwalkerTarget->IsAIHero())
		{
			if (Menu::Combo::UseQ->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo))
				Spells::Q->Cast(OrbwalkerTarget);

			if (Menu::Combo::UseR->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo))
				Spells::R->Cast(OrbwalkerTarget);
		}
	}
}


// harass
void HarassLogic()
{
	{
		if (g_LocalPlayer->ManaPercent() < Menu::Harass::ManaQ->GetInt())
			return;
	}
	if (Menu::Harass::Enabled->GetBool() && Menu::Harass::UseQ->GetBool() && !Menu::Harass::TowerQ->GetBool() && Spells::Q->IsReady())
	{
		const auto OrbwalkerTarget = g_Orbwalker->GetTarget();
		if (OrbwalkerTarget && OrbwalkerTarget->IsAIHero())
		{
			if (Menu::Harass::UseQ->GetBool() && !Menu::Harass::TowerQ->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass))
				Spells::Q->Cast(OrbwalkerTarget);
		}
	}
	if (Menu::Harass::Enabled->GetBool() && Menu::Harass::UseQ->GetBool() && Menu::Harass::TowerQ->GetBool() && Spells::Q->IsReady())
	{
		const auto OrbwalkerTarget = g_Orbwalker->GetTarget();
		if (OrbwalkerTarget && OrbwalkerTarget->IsAIHero())
		{
			if (Menu::Harass::UseQ->GetBool() && Menu::Harass::TowerQ->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass) && !g_LocalPlayer->IsUnderMyEnemyTurret())
				Spells::Q->Cast(OrbwalkerTarget);
		}
	}
}

void FleeLogic()
{
	if (Menu::Misc::UseWflee->GetBool() && Spells::W->IsReady())
	{
		Spells::W->Cast();
	}
}

// killsteal
void KillstealLogic()
{
	const auto Enemies = g_ObjectManager->GetChampions(false);
	for (auto Enemy : Enemies)
	{
		if (Menu::Killsteal::UseQ->GetBool() && Spells::Q->IsReady() && Enemy->IsInRange(Spells::Q->Range()))
		{
			auto QDamage = g_Common->GetSpellDamage(g_LocalPlayer, Enemy, SpellSlot::Q, false);
			if (Enemy->IsValidTarget() && QDamage >= Enemy->RealHealth(false, true))
				Spells::Q->Cast(Enemy);
		}
	}
}

void OnGameUpdate()
{
	// adjusting R range as you level them up
	if (Spells::R->Level() == 2)
		Spells::R->SetRange(650);
	if (Spells::R->Level() == 3)
		Spells::R->SetRange(900);

	if (g_LocalPlayer->IsDead())
		return;

	if (Menu::Combo::Enabled->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo))
		ComboLogic();

	if (Menu::Harass::Enabled->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass))
		HarassLogic();

	if (Menu::Killsteal::Enabled->GetBool())
		KillstealLogic();

	if (Menu::Misc::UseWflee->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeFlee))
		FleeLogic();
	// Q on Dash
	if (Menu::Misc::Qdash->GetBool() && Spells::Q->IsReady() && !g_LocalPlayer->IsStealthed())
	{
		const auto Enemies = g_ObjectManager->GetChampions(false);
		for (auto Enemy : Enemies)
		{
			if (Enemy->IsDashing())
			{
				const auto DashData = Enemy->GetDashData();
				if (g_LocalPlayer->Distance(DashData.EndPosition) < 430.f)
					Spells::Q->Cast(Enemy);
			}
		}
	}
	// Q on melee
	if (Menu::Misc::QantiMelee->GetBool() && Spells::Q->IsReady() && !g_LocalPlayer->IsStealthed())
	{
		const auto Enemies = g_ObjectManager->GetChampions(false);
		for (auto Enemy : Enemies)
		{
			if (Enemy->IsMelee())
			{
				if (g_LocalPlayer->Distance(Enemy) < 250.f)
					Spells::Q->Cast(Enemy);
			}
		}
	}
	// W on slow
	if (Menu::Misc::UseWslow->GetBool() && Spells::W->IsReady() && g_LocalPlayer->HasBuffOfType(BuffType::Slow))
		Spells::W->Cast();
}

// Q after AA
void OnAfterAttack(IGameObject* target)
{
	if (Menu::Combo::Enabled->GetBool() && Spells::Q->IsReady())

	{
		const auto OrbwalkerTarget = g_Orbwalker->GetTarget();
		if (OrbwalkerTarget && OrbwalkerTarget->IsAIHero())
		{
			if (Menu::Combo::UseQaa->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeCombo))
				Spells::Q->Cast(OrbwalkerTarget);
		}
	}
	{
		if (g_LocalPlayer->ManaPercent() < Menu::Harass::ManaQ->GetInt())
			return;
	}
	if (Menu::Harass::Enabled->GetBool() && Spells::Q->IsReady())
	{
		if (Menu::Harass::Enabled->GetBool() && Menu::Harass::UseQaa->GetBool() && !Menu::Harass::TowerQ->GetBool())
		{
			const auto OrbwalkerTarget = g_Orbwalker->GetTarget();
			if (OrbwalkerTarget && OrbwalkerTarget->IsAIHero())
			{
				if (Menu::Harass::UseQaa->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass))
					Spells::Q->Cast(OrbwalkerTarget);
			}
		}
	}
	if (Menu::Harass::Enabled->GetBool() && Menu::Harass::UseQaa->GetBool() && Menu::Harass::TowerQ->GetBool() && Spells::Q->IsReady())
	{
		const auto OrbwalkerTarget = g_Orbwalker->GetTarget();
		if (OrbwalkerTarget && OrbwalkerTarget->IsAIHero())
		{
			if (Menu::Harass::UseQaa->GetBool() && g_Orbwalker->IsModeActive(eOrbwalkingMode::kModeHarass) && !g_LocalPlayer->IsUnderMyEnemyTurret())
				Spells::Q->Cast(OrbwalkerTarget);
		}
	}
}

void OnProcessSpell(IGameObject* Owner, OnProcessSpellEventArgs* Args)
{
	if (Menu::Misc::Qdanger->GetBool() && Spells::Q->IsReady() && !g_LocalPlayer->IsStealthed() && g_LocalPlayer->Distance(Owner) <= 470)
	{
		if (Owner->ChampionId() == ChampionId::Darius && Args->SpellSlot == SpellSlot::W)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::KogMaw && Args->SpellSlot == SpellSlot::W)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Kaisa && Args->SpellSlot == SpellSlot::Q)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Fizz && Args->SpellSlot == SpellSlot::W)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Renekton && Args->SpellSlot == SpellSlot::W)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Gragas && Args->SpellSlot == SpellSlot::W)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Urgot && Args->SpellSlot == SpellSlot::W)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Talon && Args->SpellSlot == SpellSlot::Q)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Shyvana && Args->SpellSlot == SpellSlot::Q)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Irelia && Args->SpellSlot == SpellSlot::Q)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Sivir && Args->SpellSlot == SpellSlot::W)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Lucian && Args->SpellSlot == SpellSlot::Q)
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Warwick && Args->SpellSlot == SpellSlot::R)
			Spells::Q->Cast(Owner);
	}
}
// Q on Dangerous abilities On Buff
void OnBuffChange(IGameObject* Owner, OnBuffEventArgs* Args)
{
	if (Menu::Misc::Qdanger->GetBool() && Spells::Q->IsReady() && !g_LocalPlayer->IsStealthed() && g_LocalPlayer->Distance(Owner) <= 670)
	{
		if (Owner->ChampionId() == ChampionId::TwistedFate && Owner->HasBuff(hash("GoldCardPreAttack")))
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Lucian && Owner->HasBuff(hash("LucianPassiveShot")))
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::MasterYi && Owner->HasBuff(hash("MasterYiDoubleStrike")))
			Spells::Q->Cast(Owner);

		if (Owner->ChampionId() == ChampionId::Jayce && Owner->HasBuff(hash("JayceHyperCharge")))
			Spells::Q->Cast(Owner);
	}
}

// drawings
void OnHudDraw()
{
	if (!Menu::Drawings::Toggle->GetBool() || g_LocalPlayer->IsDead())
		return;

	const auto PlayerPosition = g_LocalPlayer->Position();
	const auto CirclesWidth = 1.5f;

	if (Menu::Drawings::DrawQRange->GetBool() && !Spells::Q->CooldownTime())
		g_Drawing->AddCircle(PlayerPosition, Spells::Q->Range(), Menu::Colors::QColor->GetColor(), CirclesWidth);

	if (Menu::Drawings::DrawRRange->GetBool() && Spells::R->Level())
		g_Drawing->AddCircle(PlayerPosition, Spells::R->Range(), Menu::Colors::RColor->GetColor(), CirclesWidth);
}

PLUGIN_API bool OnLoadSDK(IPluginsSDK* plugin_sdk)
{
	DECLARE_GLOBALS(plugin_sdk);

	if (g_LocalPlayer->ChampionId() != ChampionId::Teemo)
		return false;

	using namespace Menu;
	using namespace Spells;

	MenuInstance = g_Menu->CreateMenu("Teemo", "Teemo by Biggie");

	const auto ComboSubMenu = MenuInstance->AddSubMenu("Combo", "combo_menu");
	Menu::Combo::Enabled = ComboSubMenu->AddCheckBox("Enable Combo", "enable_combo", true);
	Menu::Combo::UseQ = ComboSubMenu->AddCheckBox("Use Q", "combo_use_q", false);
	Menu::Combo::UseQaa = ComboSubMenu->AddCheckBox("Use Q after AA", "combo_use_qaa", false);
	Menu::Combo::UseR = ComboSubMenu->AddCheckBox("Use R", "combo_use_R", false);

	const auto HarassSubMenu = MenuInstance->AddSubMenu("Harass", "harass_menu");
	Menu::Harass::Enabled = HarassSubMenu->AddCheckBox("Enable Harass", "enable_harass", true);
	Menu::Harass::UseQ = HarassSubMenu->AddCheckBox("Use Q", "harass_use_q", false);
	Menu::Harass::UseQaa = HarassSubMenu->AddCheckBox("Use Q after AA", "harass_use_qaa", true);
	Menu::Harass::TowerQ = HarassSubMenu->AddCheckBox("Don't Use Q under tower", "use_tower_q", true);
	Menu::Harass::ManaQ = HarassSubMenu->AddSlider("Min Mana", "min_mana_harass", 50, 0, 100, true);

	const auto KSSubMenu = MenuInstance->AddSubMenu("KS", "ks_menu");
	Menu::Killsteal::Enabled = KSSubMenu->AddCheckBox("Enable Killsteal", "enable_ks", true);
	Menu::Killsteal::UseQ = KSSubMenu->AddCheckBox("Use Q", "q_ks", true);

	const auto MiscSubMenu = MenuInstance->AddSubMenu("Misc", "misc_menu");
	Menu::Misc::UseWflee = MiscSubMenu->AddCheckBox("Use W in Flee", "w_flee", true);
	Menu::Misc::UseWslow = MiscSubMenu->AddCheckBox("Auto W on Slows", "w_slow", true);
	Menu::Misc::Qdanger = MiscSubMenu->AddCheckBox("Q Dangerous Abilties", "q_danger", true);
	Menu::Misc::QantiMelee = MiscSubMenu->AddCheckBox("Anti-melee Q", "q_antimelee", true);
	Menu::Misc::Qdash = MiscSubMenu->AddCheckBox("Antigapcloser", "q_dashing", true);

	const auto DrawingsSubMenu = MenuInstance->AddSubMenu("Drawings", "drawings_menu");
	Drawings::Toggle = DrawingsSubMenu->AddCheckBox("Enable Drawings", "drawings_toggle", true);
	Drawings::DrawQRange = DrawingsSubMenu->AddCheckBox("Draw Q Range", "draw_q", true);
	Drawings::DrawRRange = DrawingsSubMenu->AddCheckBox("Draw R Range", "draw_r", true);

	const auto ColorsSubMenu = DrawingsSubMenu->AddSubMenu("Colors", "color_menu");
	Colors::QColor = ColorsSubMenu->AddColorPicker("Q Range", "color_q_range", 0, 175, 255, 180);
	Colors::RColor = ColorsSubMenu->AddColorPicker("R Range", "color_r_range", 200, 200, 200, 180);

	Spells::Q = g_Common->AddSpell(SpellSlot::Q, 680.f);
	Spells::W = g_Common->AddSpell(SpellSlot::W);
	Spells::E = g_Common->AddSpell(SpellSlot::E);
	Spells::R = g_Common->AddSpell(SpellSlot::R, 400.f);

	// pred hitchance is very good with these weird values
	//Spells::Q->CollisionFlags() == kCollidesWithYasuoWall)

	EventHandler<Events::OnProcessSpellCast>::AddEventHandler(OnProcessSpell);
	EventHandler<Events::OnBuff>::AddEventHandler(OnBuffChange);
	EventHandler<Events::GameUpdate>::AddEventHandler(OnGameUpdate);
	EventHandler<Events::OnAfterAttackOrbwalker>::AddEventHandler(OnAfterAttack);
	EventHandler<Events::OnHudDraw>::AddEventHandler(OnHudDraw);

	g_Common->ChatPrint("<font color='#FFC300'>Teemo Loaded!</font>");
	g_Common->Log("Teemo plugin loaded.");

	return true;
}

PLUGIN_API void OnUnloadSDK()
{
	Menu::MenuInstance->Remove();
	EventHandler<Events::GameUpdate>::RemoveEventHandler(OnGameUpdate);
	EventHandler<Events::OnProcessSpellCast>::RemoveEventHandler(OnProcessSpell);
	EventHandler<Events::GameUpdate>::RemoveEventHandler(OnGameUpdate);
	EventHandler<Events::OnAfterAttackOrbwalker>::RemoveEventHandler(OnAfterAttack);
	EventHandler<Events::OnHudDraw>::RemoveEventHandler(OnHudDraw);
	EventHandler<Events::OnBuff>::RemoveEventHandler(OnBuffChange);

	g_Common->ChatPrint("<font color='#00BFFF'>Teemo Unloaded.</font>");
}