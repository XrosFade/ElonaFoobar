local Chara = Elona.require("Chara")
local Math = Elona.require("Math")
local I18N = Elona.require("I18N")

local function mod_skill_level(args, id, amount)
   local skill = args.chara:get_skill(id)

   skill.current_level = skill.current_level + amount
end

local function mod_skill_level_clamp(args, id, amount)
   local skill = args.chara:get_skill(id)

   skill.current_level =
      Math.clamp(skill.current_level + amount,
                 skill.current_level and 1 or 0, 9999)
end

local function get_description(self, power)
   return I18N.get_enum_property("core.locale.buff", "description", self.id, self._effect(power))
end

--[[
List of fields:

buff_type:
   indicates if the buff has a positive or negative
   effect. If the buff is of type "Hex" and is obtained from an item,
   the buff will gain more power if the item is cursed (power = power
   * 150 / 100), and less if it is blessed (power = 50). Vice-versa
   for the other buff types.
      - If buff_type is "Buff", the buff animation is played on gain.
      - If the buff type is "Food", it will be lost if the bearer vomits.
      - If buff_type is "Hex":
         + The debuff animation is played on gain.
         + Any instance of it will be considered a target for removal in
           Holy Light/Vanquish Hex.
         + The target of the buff can potentially resist gaining it
           through skills/traits.
duration:
   function taking a power level which returns the number of turns the
   buff lasts.
on_refresh:
   function for applying the effects of the buff. The effects are
   applied on character refresh, so the state of the character will be
   reset before applying. It takes two arguments, "self" which is the
   buff definition itself and "args", a table with these fields:
      - power: buff power.
      - chara: character which the buff is being applied to.
on_removal:
   function which is run when the buff expires. It takes two
   arguments, "self" and the "args" table, with these fields:
      - chara: character which the buff was applied to.
description:
   function for returning the localized buff description. It takes two
   arguments, "self" and "power", the buff's power. It can be used for
   passing additional arguments to be used in the localized string,
   primarily the buff's calculated power.

_effect isn't strictly necessary, but it is typically used for
calculating the actual effect from the buff's power, and displaying it
in the description.
]]

-- TODO: buff icons
data:define_type("buff")
data:add_multi(
   "core.buff",
   {
      {
         -- NOTE: Has these hardcoded behaviors.
         --  + Attempts to apply fear will be ignored.
         name = "holy_shield",
         id = 1,
         buff_type = "Buff",
         duration = function(power)
            return 10 + power // 10
         end,
         on_refresh = function(self, args)
            args.chara.pv = args.chara.pv + self._effect(args.power)
            args.chara:heal_ailment("Fear", 0)
         end,
         _effect = function(power)
            return 25 + power // 15
         end,
         description = get_description
      },
      {
         -- NOTE: Has these hardcoded behaviors.
         --  + Silence behavior.
         name = "mist_of_silence",
         id = 2,
         buff_type = "Hex",
         duration = function(power)
            return 5 + power // 40
         end,
         on_refresh = function(self, args)
         end,
         _effect = function(power)
            return 0
         end,
         description = get_description
      },
      {
         name = "regeneration",
         id = 3,
         buff_type = "Buff",
         duration = function(power)
            return 12 + power // 20
         end,
         on_refresh = function(self, args)
            mod_skill_level(args, 154, 40)
         end,
         _effect = function(power)
            return 0
         end,
         description = get_description
      },
      {
         name = "elemental_shield",
         id = 4,
         buff_type = "Buff",
         duration = function(power)
            return 4 + power // 6
         end,
         on_refresh = function(self, args)
            mod_skill_level(args, 50, 100)
            mod_skill_level(args, 51, 100)
            mod_skill_level(args, 52, 100)
         end,
         _effect = function(power)
            return 0
         end,
         description = get_description
      },
      {
         name = "speed",
         id = 5,
         buff_type = "Buff",
         duration = function(power)
            return 8 + power // 30
         end,
         on_refresh = function(self, args)
            mod_skill_level(args, 18, self._effect(args.power))
         end,
         _effect = function(power)
            return Math.modf(50 + Math.sqrt(power // 5))
         end,
         description = get_description
      },
      {
         name = "slow",
         id = 6,
         buff_type = "Hex",
         duration = function(power)
            return 8 + power // 30
         end,
         on_refresh = function(self, args)
            args.chara:get_skill(18).current_level =
               args.chara:get_skill(18).current_level - self._effect(args.power)
         end,
         _effect = function(power)
            return Math.min(20 + power // 20, 50)
         end,
         description = get_description
      },
      {
         -- NOTE: Has these hardcoded behaviors.
         --  + Attempts to apply confusion or fear will be ignored.
         name = "hero",
         id = 7,
         buff_type = "Buff",
         duration = function(power)
            return 10 + power // 4
         end,
         on_refresh = function(self, args)
            mod_skill_level(args, 10, self._effect(args.power))
            mod_skill_level(args, 12, self._effect(args.power))
            args.chara:heal_ailment("Fear", 0)
            args.chara:heal_ailment("Confused", 0)
         end,
         _effect = function(power)
            return 5 + power // 30
         end,
         description = get_description
      },
      {
         name = "mist_of_frailness",
         id = 8,
         buff_type = "Hex",
         duration = function(power)
            return 6 + power // 10
         end,
         on_refresh = function(self, args)
            args.chara.dv = args.chara.dv // 2
            args.chara.pv = args.chara.pv // 2
         end,
         _effect = function(power)
            return 0
         end,
         description = get_description
      },
      {
         name = "element_scar",
         id = 9,
         buff_type = "Hex",
         duration = function(power)
            return 4 + power // 15
         end,
         on_refresh = function(self, args)
            mod_skill_level_clamp(args, 50, -100)
            mod_skill_level_clamp(args, 51, -100)
            mod_skill_level_clamp(args, 52, -100)
         end,
         _effect = function(power)
            return 0
         end,
         description = get_description
      },
      {
         -- NOTE: Has these hardcoded behaviors.
         --  + Additional chance to resist if a hex is applied to this
         --    character.
         name = "holy_veil",
         id = 10,
         buff_type = "Buff",
         duration = function(power)
            return 15 + power // 5
         end,
         on_refresh = function(self, args)
         end,
         _effect = function(power)
            return 50 + power // 3 * 2
         end,
         description = get_description
      },
      {
         name = "nightmare",
         id = 11,
         buff_type = "Hex",
         duration = function(power)
            return 4 + power // 15
         end,
         on_refresh = function(self, args)
            mod_skill_level_clamp(args, 58, -100)
            mod_skill_level_clamp(args, 54, -100)
         end,
         _effect = function(power)
            return 0
         end,
         description = get_description
      },
      {
         name = "divine_wisdom",
         id = 12,
         buff_type = "Buff",
         duration = function(power)
            return 10 + power // 4
         end,
         on_refresh = function(self, args)
            local a, b = self._effect(args.power)
            mod_skill_level(args, 14, a)
            mod_skill_level(args, 16, a)
            mod_skill_level(args, 150, b)
         end,
         _effect = function(power)
            return 6 + power // 40, 3 + power // 100
         end,
         description = get_description
      },
      {
         -- NOTE: Has these hardcoded behaviors.
         --  + Ignored when removing status effects on a character.
         --  + Ignored when casting Holy Light/Vanquish Hex.
         name = "punishment",
         id = 13,
         buff_type = "Hex",
         duration = function(power)
            return power
         end,
         on_refresh = function(self, args)
            mod_skill_level(args, 18, -self._effect(args.power))
            if args.chara.pv > 1 then
               args.chara.pv = args.chara.pv // 5
            end
         end,
         _effect = function(power)
            return 20, 20
         end,
         description = get_description
      },
      {
         name = "lulwys_trick",
         id = 14,
         buff_type = "Buff",
         duration = function(power)
            return 7
         end,
         on_refresh = function(self, args)
            mod_skill_level(args, 18, self._effect(args.power))
         end,
         _effect = function(power)
            return 155 + power // 5
         end,
         description = get_description
      },
      {
         -- NOTE: The initial incognito effect is applied by the
         -- incognito spell when it is cast, but the effect when the
         -- buff expires is handled by the buff itself.
         name = "incognito",
         id = 15,
         buff_type = "Buff",
         duration = function(power)
            return 4 + power // 40
         end,
         on_refresh = function(self, args)
            args.chara:set_flag("IsIncognito", true)
         end,
         on_removal = function(self, args)
            if not Chara.is_player(args.chara) then
               return
            end
            for _, chara in Chara.iter(16, 245) do
               if Chara.is_alive(chara) then
                  if chara.role == 14 and Chara.player().karma < -30 then
                     chara.relationship = "Aggressive"
                     chara.hate = 80
                     chara.emotion_icon = 218
                  end
               end
            end
         end,
         _effect = function(power)
            return 0
         end,
         description = get_description
      },
      {
         -- NOTE: Has these hardcoded behaviors.
         --  + Inflicts 9999 damage when it expires, but outside of
         --    buff_delete().
         --  + On application, will always be resisted if character is
         --    of "Miracle" or "Godly" quality.
         --  + Removed when a character with the "IsDeathMaster" flag
         --    is killed.
         name = "death_word",
         id = 16,
         buff_type = "Hex",
         duration = function(power)
            return 20
         end,
         on_refresh = function(self, args)
            args.chara:set_flag("IsSentencedDaeth", true)
         end,
         on_removal = function(self, args)
            args.chara:set_flag("IsSentencedDaeth", false)
         end,
         _effect = function(power)
            return 0
         end,
         description = get_description
      },
      {
         name = "boost",
         id = 17,
         buff_type = "Buff",
         duration = function(power)
            return 5
         end,
         on_refresh = function(self, args)
            mod_skill_level(args, 18, self._effect(args.power))
            args.chara:get_skill(10).current_level = args.chara:get_skill(10).current_level * 150 // 100 + 10
            args.chara:get_skill(12).current_level = args.chara:get_skill(12).current_level * 150 // 100 + 10
            mod_skill_level(args, 154, 50)
            args.chara.pv = args.chara.pv * 150 // 100 + 25
            args.chara.dv = args.chara.dv * 150 // 100 + 25
            args.chara.hit_bonus = args.chara.hit_bonus * 150 // 100 + 50
         end,
         _effect = function(power)
            return 120
         end,
         description = get_description
      },
      {
         -- NOTE: Has these hardcoded behaviors.
         --  + Check for lethal damage and chance to heal. If the
         --    "IsContractingWithReaper" flag is set then the buff is
         --    expected to be be active on the same character.
         name = "contingency",
         id = 18,
         buff_type = "Buff",
         duration = function(power)
            return 66
         end,
         on_refresh = function(self, args)
            args.chara:set_flag("IsContractingWithReaper", true)
         end,
         on_removal = function(self, args)
            args.chara:set_flag("IsContractingWithReaper", false)
         end,
         _effect = function(power)
            return Math.clamp(25 + power // 17, 25, 80)
         end,
         description = get_description
      },
      {
         name = "luck",
         id = 19,
         buff_type = "Buff",
         duration = function(power)
            return 777
         end,
         on_refresh = function(self, args)
            mod_skill_level(args, 19, self._effect(args.power))
         end,
         _effect = function(power)
            return power
         end,
         description = get_description
      },
   }
)

local function register_growth_buff(attribute_index, name)
   data:add({
      {
         type = "core.buff",
         name = "grow_" .. name,
         id = attribute_index + 20,
         buff_type = "Food",
         duration = function(power)
            return 10 + power // 10
         end,
         on_refresh = function(self, args)
            args.chara:set_growth_buff(attribute_index, self._effect(args.power))
         end,
         _effect = function(power)
            return power
         end,
         description = get_description
      }
   })
end


-- These buffs will be applied when a piece of equipment with a
-- corresponding attribute preservation enchantment is eaten. The buff
-- applied depends on the integer ID of the preserved attribute, so
-- only the attribute IDs from 0-9 are valid.
register_growth_buff(0, "strength")
register_growth_buff(1, "constitution")
register_growth_buff(2, "dexterity")
register_growth_buff(3, "perception")
register_growth_buff(4, "learning")
register_growth_buff(5, "will")
register_growth_buff(6, "magic")
register_growth_buff(7, "charisma")
register_growth_buff(8, "speed")
register_growth_buff(9, "luck")
