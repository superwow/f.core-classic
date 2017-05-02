/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Moam
SD%Complete: 100
SDComment:
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "AI/ScriptDevAI/include/precompiled.h"

enum
{
    EMOTE_AGGRO              = -1509000,
    EMOTE_MANA_FULL          = -1509001,
    EMOTE_ENERGIZING         = -1509028,

    SPELL_TRAMPLE            = 15550,
    SPELL_ARCANE_ERUPTION    = 25672,
    SPELL_DRAIN_MANA         = 25676,
    SPELL_SUMMON_MANAFIENDS  = 25684,
    SPELL_ENERGIZE           = 25685,
};

struct boss_moamAI : public ScriptedAI
{
    boss_moamAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiTrampleTimer;
    uint32 m_uiManaDrainTimer;
    uint32 m_uiEnergizeTimer;
    uint32 m_uiAddCount;

    void Reset() override
    {
        m_uiTrampleTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        m_uiManaDrainTimer = 6 * IN_MILLISECONDS;
        m_uiEnergizeTimer = 1.5 * MINUTE * IN_MILLISECONDS;
        m_uiAddCount = 0;
        m_creature->SetPower(POWER_MANA, 0);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(EMOTE_AGGRO, m_creature);
    }

    void JustSummoned(Creature* /*pSummoned*/) override
    {
        m_uiAddCount++;
    }

    void SummonedCreatureJustDied(Creature* /*pSummmoned*/) override
    {
        m_uiAddCount--;

        if (m_uiAddCount == 0)
        {
            m_creature->RemoveAurasDueToSpell(SPELL_ENERGIZE);

            m_uiEnergizeTimer = 1.5 * MINUTE * IN_MILLISECONDS;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Trample
        if (m_uiTrampleTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TRAMPLE) == CAST_OK)
                m_uiTrampleTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
        }
        else
            m_uiTrampleTimer -= uiDiff;

        // Mana Drain
        if (m_uiManaDrainTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_DRAIN_MANA) == CAST_OK)
                m_uiManaDrainTimer = 6 * IN_MILLISECONDS;
        }
        else
            m_uiManaDrainTimer -= uiDiff;

        // Energize
        if (m_uiEnergizeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_MANAFIENDS) == CAST_OK)
                if (DoCastSpellIfCan(m_creature, SPELL_ENERGIZE) == CAST_OK)
                {
                    DoScriptText(EMOTE_ENERGIZING, m_creature);
                    m_uiEnergizeTimer = 0;
                }
        }
        else
            m_uiEnergizeTimer -= uiDiff;

        // Arcane Eruption
        if (m_creature->GetPowerPercent() == 100.0f)
        {
            m_creature->RemoveAurasDueToSpell(SPELL_ENERGIZE);

            if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_ERUPTION) == CAST_OK)
            {
                DoScriptText(EMOTE_MANA_FULL, m_creature);
                m_uiEnergizeTimer = 1.5 * MINUTE * IN_MILLISECONDS;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_moam(Creature* pCreature)
{
    return new boss_moamAI(pCreature);
}

void AddSC_boss_moam()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_moam";
    pNewScript->GetAI = &GetAI_boss_moam;
    pNewScript->RegisterSelf();
}
