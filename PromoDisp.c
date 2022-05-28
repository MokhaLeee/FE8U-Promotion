#include "gbafe.h"
#include "_Promotion.h"




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
	sub_80CCBF4();
	
	extern void sub_80CD408(int, int, int);
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
	
	proc->unk_40 = 1;
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