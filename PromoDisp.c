#include "gbafe.h"
#include "_Promotion.h"

struct Proc_PromoDispSub{
	PROC_HEADER;
	u16 promotedClassIds[7];
	u16 promotedClassAnimIds[7];
	u16 promotedClassDescIds[7];
	u16 promotedClassHasAnim[7];
};


u16 PromoDispMainLoopHackCore(struct Proc_PromoDisp* proc){
	
	// Hack from 80CD0A8, return at 0x80CD105
	// return value: GetBattleAnimationId
	
	u32 wType;
	u16 charId = proc->charId;
	u16 classId = proc->promotedClassIds[proc->selectedOptionIndex];
	struct Unit *unit = GetUnitFromCharId(charId);
	const struct ClassData *class_promo = GetClassData(classId);
	const u8 *AnimDef = class_promo->pBattleAnimDef;
	
	// GetBattleAnimationId
	extern u16 GetBattleAnimationId(struct Unit*, const void* pBattleAnimDef, u16 weapon, u32 *R3);
	
	u16 anime_id = GetBattleAnimationId(
		unit, 
		AnimDef, 
		GetUnitEquippedWeapon(unit), 
		&wType);
	
	if( 0xFFFF == anime_id )
		if( 0 != AnimDef[2] )
		{
			anime_id = AnimDef[2];
			wType = AnimDef[3];
		}
	
	// Find a Displayed weapon
	for( int i = 1; i < 0xFF; i++ )
		if( GetItemType(i) == wType )
		{
			proc->displayedWeapon = i;
			break;
		}
	
	// to transmit values to asm
	gGenericBuffer[0] = charId - 1;
	gGenericBuffer[1] = classId;
	
	return anime_id;
	
}



// Vanilla rework
void PromotionDisplay_Init(struct Proc_PromoDisp* proc){
	
	struct Proc_PromoMain* parent = proc->proc_parent;
	
	parent->unk_29 = 2;
	proc->charId = parent->charId;
	proc->unk_50 = 9;
	
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);
	
	LoadUiFrameGraphics();
	LoadObjUIGfx();
	
	// Maybe relate to Ekr Anim
	extern void sub_80CD47C(int, int, int, int, int);
	sub_80CD47C(0, -1, 0x1F6, 0x58, 0x6);
	
	// Maybe uncompress some gfx
	extern void sub_80CCBF4();
	extern void sub_80CD408(int, int, int);
	sub_80CCBF4();
	sub_80CD408(proc->unk_50, 0x118, 0x68);
	
	
	proc->promotedClassAnimIds[0] = 0;
	proc->promotedClassAnimIds[1] = 0;
	proc->promotedClassAnimIds[2] = 0;
	
	// In vanilla, these two value is not init
	// I think it is a bug!
	u8 class_id = 0;
	u16 weapon = 0;
	
	for( int i = 1; i < FACTION_GREEN; i++ ){
		
		struct Unit* unit = GetUnit(i);
		
		if( UNIT_IS_VALID(unit) && (unit->pCharacterData->number == proc->charId) ){
			
			class_id = unit->pClassData->number;
			weapon = GetUnitEquippedWeapon(unit);
			// class_id = 0x13;

			for(int i = 0; i <= 1; i++){
				proc->promotedClassIds[i] = gPromotionTable[class_id].promoClass[i];
				
				extern u8 GetClassAnimationIdForWeapon(u16* AnimIdAt, u8 classId, u16 weapon);
				proc->promotedClassHasAnim[i] = 
					GetClassAnimationIdForWeapon(&proc->promotedClassAnimIds[i], class_id, weapon);
				
				proc->promotedClassDescIds[i] = 
					GetClassData(gPromotionTable[class_id].promoClass[i])->descTextId;
				
			}
			
			proc->displayedWeapon = weapon;
			
			// Check some Trainee
			
			break;
		}
	}
	
	
	// ???
	if( 0 == proc->promotedClassAnimIds[0] )
		if( 0 == proc->promotedClassAnimIds[1] ){
			proc->promotedClassAnimIds[0] = 0;
			proc->promotedClassAnimIds[1] = 0;
		}
	
	proc->state = 1;
	proc->selectedOptionIndex = 0;
	
	// This should be class UI name
	extern void PromotionDisplay_InitClassName(struct Proc_PromoDisp*, u8 classId);
	extern void PromotionDisplay_DrawClassName(struct Proc_PromoDisp*);
	extern struct MenuProc* Make6C_PromotionMenuSelect(struct Proc_PromoDisp*);
	
	PromotionDisplay_InitClassName(proc, class_id);
	PromotionDisplay_DrawClassName(proc);
	LoadObjUIGfx();
	proc->pMenuSelectProc = Make6C_PromotionMenuSelect(proc);
	
	struct Proc_PromoInit* GrandFather = parent->proc_parent;
	
	if( 1 == GrandFather->promContextId ){
		
		extern void RestartScreenMenuScrollingBg();
		RestartScreenMenuScrollingBg();
		BG_EnableSyncByMask(0b1111);
	}
}











// =========================================================
//                             Menu
// =========================================================

static u8 NewPromoCmd_Usability(const struct MenuItemDef*, int number);
static int NewPromoCmd_DrawText(struct MenuProc*, struct MenuItemProc*);
static u8 NewPromoCmd_Effect(struct MenuProc*, struct MenuItemProc*);
static int NewPromoCmd_Hover(struct MenuProc*, struct MenuItemProc*);

const struct MenuItemDef MenuItems_NewPromotion[] = {
	
	#define PROMO_SELECT_ITEM(i) { \
		.name = 0, \
		.nameMsgId = 0, \
		.helpMsgId = 0, \
		.color = TEXT_COLOR_NORMAL, \
		.overrideId = i, \
		.isAvailable = NewPromoCmd_Usability, \
		.onDraw = NewPromoCmd_DrawText, \
		.onSelected = NewPromoCmd_Effect, \
		.onIdle = 0, \
		.onSwitchIn = NewPromoCmd_Hover, \
	}
	
	[0] = PROMO_SELECT_ITEM(0),
	[1] = PROMO_SELECT_ITEM(1),
	[2] = PROMO_SELECT_ITEM(2),
	[3] = PROMO_SELECT_ITEM(3),
	[4] = PROMO_SELECT_ITEM(4),
	[5] = {0}
	
	#undef PROMO_SELECT_ITEM
};


u8 NewPromoCmd_Usability(const struct MenuItemDef*, int number){
	
	return number < 2 ? MENU_ENABLED : MENU_NOTSHOWN;
}


int NewPromoCmd_DrawText(struct MenuProc* menu, struct MenuItemProc* menu_item){
	
	static void (*DrawClassNamePromoMenuItem)(struct MenuProc*, struct MenuItemProc*, char* str) = (const void*)0x80CDC49;
	
	struct Proc_PromoMenuSelect* parent = menu->proc_parent;
	struct Proc_PromoDisp* GrandFa = parent->proc_parent;
	u16 classId = GrandFa->promotedClassIds[menu_item->itemNumber];
	const struct ClassData *class = GetClassData(classId);
	
	DrawClassNamePromoMenuItem(
		menu,
		menu_item,
		GetStringFromIndex( class->nameTextId )
	);
	
	return 0;
}



u8 NewPromoCmd_Effect(struct MenuProc* menu, struct MenuItemProc* menu_item){
	
	struct Proc_PromoMenuSelect* parent = menu->proc_parent;
	struct Proc_PromoDisp* procDisp = parent->proc_parent;
	struct Proc_PromoMain* procMain = procDisp->proc_parent;
	
	if( 0 != procDisp->state )
		return 0;
	
	struct Unit* unit = GetUnitFromCharId(procDisp->charId);
	procMain->promoClass = procDisp->promotedClassIds[menu_item->itemNumber];
	
	switch(procMain->promoClass){
		case CLASS_RANGER:
		case CLASS_RANGER_F:
			TryRemoveUnitFromBallista(unit);
			break;
			
		default:
			break;
	}
	
	EndMenu(menu);
	
	extern void EndAllProcChildren(ProcPtr);
	extern void sub_80CCBD4();
	
	EndAllProcChildren(procMain);
	sub_80CCBD4();
	
	Proc_Goto(procMain, 0x5);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}



int NewPromoCmd_Hover(struct MenuProc* menu, struct MenuItemProc* menu_item){
	struct Proc_PromoMenuSelect* parent = menu->proc_parent;
	struct Proc_PromoDisp* procDisp = parent->proc_parent;
	extern void ChangePromotionClassDescText(u16 msg);
	extern void Dialogue_SetCharacterDisplayDelay(int time);
	
	procDisp->state = 1;
	procDisp->selectedOptionIndex = menu_item->itemNumber;
	ChangePromotionClassDescText( procDisp->promotedClassDescIds[menu_item->itemNumber] );
	Dialogue_SetCharacterDisplayDelay(-1);
	return 0;
}