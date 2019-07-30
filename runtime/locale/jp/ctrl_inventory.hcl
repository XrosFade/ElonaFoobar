locale {
    ui {
        inv {
            title {
                # _0
                _1 = "どのアイテムを調べる？ "
                _2 = "どのアイテムを置く？ "
                _3 = "どのアイテムを拾う？ "
                _4 = "何を装備する？"
                _5 = "何を食べよう？ "
                # _6
                _7 = "どれを読む？ "
                _8 = "どれを飲む？ "
                _9 = "どれを振る？ "
                _10 = "どれを渡す？ "
                _11 = "どれを購入する？ "
                _12 = "どれを売却する？ "
                _13 = "どのアイテムを鑑定する？ "
                _14 = "どのアイテムを使用する？ "
                _15 = "どれを開ける？ "
                _16 = "何を料理する？ "
                _17 = "何を混ぜる？ "
                _18 = "何に混ぜる？(${_1}の効果を適用するアイテムを選択) "
                _19 = "何を神に捧げる？ "
                _20 = "何を交換する？ "
                _21 = "${_1}の代わりに何を提示する？ "
                _22 = "何を取る？ "
                _23 = "何を対象にする？ "
                _24 = "何を入れる？ "
                _25 = "何をもらう？ "
                _26 = "何を投げる？ "
                _27 = "何を盗む？ "
                _28 = "何と交換する？ "
                _29 = "何を予約する？"
            }

            window {
                tag {
                    no_drop = "保持指定"
                    multi_drop = "連続で置く"
                }

                change = "メニュー切替"
                select_item = "${_1}アイテムの選択"

                weight = "重さ"
                name = "アイテムの名称"

                total_weight = "重さ合計 ${_1}/${_2}  荷車 ${_3}"

                ground = "足元"
                main_hand = "利腕"
            }

            common {
                set_as_no_drop = "それはあなたの大事なものだ。<調べる>メニューから解除できる。"
                does_not_exist = "そのアイテムは存在しない。"
                inventory_is_full = "バックパックが一杯だ。"

                invalid = "Invalid Item Id found. Item No:${_1}, Id:${_2} has been removed from your inventory."

                shortcut {
                    cargo = "荷車の荷物は登録できない。"
                }
            }

            examine {
                no_drop {
                    set = "${itemname(_1)}を大事なものに指定した。"
                    unset = "${itemname(_1)}はもう大事なものではない。"
                }
            }

            drop {
                multi = "続けてアイテムを置くことができる。"
                cannot_anymore = "これ以上は置けない。"
                how_many = "${itemname(_2, 1)}をいくつ落とす？ (1〜${_1}) "
            }

            eat {
                too_bloated = [
                    "今はとても食べられない。",
                    "腹がさける…",
                    "まだ腹は減っていない。"
                ]
            }

            equip {
                too_heavy = "それは重すぎて装備できない。"

                you_equip = "${itemname(_1)}を装備した。"
                doomed = "${name(_1)}は破滅への道を歩み始めた。"
                cursed = "${name(_1)}は急に寒気がしてふるえた。"
                blessed = "${name(_1)}は何かに見守られている感じがした。"
            }

            identify {
                need_more_power = "新しい知識は得られなかった。より上位の鑑定で調べる必要がある。"
                partially = "それは${itemname(_1)}だと判明したが、完全には鑑定できなかった。"
                fully = "それは${itemname(_1)}だと完全に判明した。"
            }

            give {
                is_sleeping = "${name(_1)}は眠っている。"
                inventory_is_full = "${name(_1)}はこれ以上持てない。"

                present {
                    text = "あなたは${name(_1)}に${itemname(_2, 1)}をプレゼントした。"
                    dialog = "「え、これを${ore(_1, 3)}にくれるの${ka(_1, 1)}${thanks(_1, 2)}」"
                }

                refuse_dialog {
                    _0 = "${name(_1)}「重すぎ」"
                    _1 = "${name(_1)}「無理」"
                    _2 = "${name(_1)}「いらん」"
                    _3 = "${name(_1)}「イヤ！」"
                }

                too_creepy = "「そんな得体の知れないものはいらない${yo(_1)}」"
                cursed = "「それ、呪われてい${ru(_1)}」"
                no_more_drink = "「もう飲めない${yo(_1)}」"
                abortion = "「おろす…」"

                you_hand = "${itemname(_1, 1)}を渡した。"
                engagement = "${name(_1)}は顔を赤らめた。"

                love_potion {
                    text = "${name(_1)}は激怒して${itemname(_2, 1)}を叩き割った。"
                    dialog = [
                        "${name(_1)}「サイテー！！」",
                        "${name(_1)}「このヘンタイ！」",
                        "${name(_1)}「ガード！ガード！ガード！」"
                    ]
                }

                refuses = "${name(_1)}は${itemname(_2, 1)}を受け取らない。"
            }

            buy {
                how_many = "${itemname(_2, 1)}をいくつ買う？ (1〜${_1})"
                prompt = "${_1}を ${_2} gp で買う？"

                not_enough_money = [
                    "あなたは財布を開いてがっかりした…",
                    "もっと稼がないと買えない！"
                ]

                window {
                    price = "値段"
                }
            }

            sell {
                how_many = "${itemname(_2, 1)}をいくつ売る？ (1〜${_1})"
                prompt = "${_1}を ${_2} gp で売る？"

                not_enough_money = "${name(_1)}は財布を開いてがっかりした…"
            }

            offer {
                no_altar = "ここには祭壇がない。"
            }

            trade {
                too_low_value = "${itemname(_1)}に見合う物を所持していない。"
                you_receive = "${itemname(_2)}を${itemname(_1)}と交換した。"
            }

            take {
                no_claim = "遺産の相続権を持っていない。"
                can_claim_more = "残り${_1}個分のアイテムの相続権を持っている。"
                really_leave = "まだアイテムが残っているがいい？"
            }

            put {
                container {
                    full = "これ以上入らない。"
                    too_heavy = "重さが${_1}以上の物は入らない。"
                    cannot_hold_cargo = "荷物は入らない。"
                }

                guild {
                    have_no_quota = "現在魔術士ギルドのノルマはない。"
                    you_deliver = "${itemname(_1)}を納入した"
                    remaining = "ノルマ残り: ${_1}GP"
                    you_fulfill = "ノルマを達成した！"
                }

                tax {
                    not_enough_money = "金が足りない…"
                    do_not_have_to = "まだ納税する必要はない。"
                    you_pay = "${itemname(_1)}を支払った。"
                }

                deck = "${itemname(_1, 1)}をデッキに加えた。"

                harvest = "${itemname(_1)}を納入した。 +${_2}  納入済み(${_3}) 納入ノルマ(${_4})"
            }

            take_ally {
                refuse_dialog = "${name(_1)}「あげないよ」"

                cursed = "${itemname(_1)}は呪われていて外せない。"
                swallows_ring = "${name(_1)}は激怒して${itemname(_2, 1)}を飲み込んだ。"
                you_take = "${_1}を受け取った。"

                window {
                    equip_weight = "装備重量"
                    equip = "装備箇所"
                }
            }

            throw {
                cannot_see = "その場所は見えない。"
                location_is_blocked = "そこには投げられない。"
            }

            steal {
                has_nothing = "${name(_1)}は盗めるものを所持していない。"
                there_is_nothing = "そこに盗めるものはない。"
                do_not_rob_ally = "仲間からは盗みたくない。"
            }

            trade_medals {
                medals = "(持っているメダル: ${_1}枚)"
                medal_value = "${_1} 枚"
                inventory_full = "これ以上持てない。"
                not_enough_medals = "メダルの数が足りない…"
                you_receive = "${itemname(_1, 1)}を受け取った！"

                window {
                    medal = "メダル"
                }
            }

            cannot_use_cargo_items = "荷車の荷物は街か野外でしか操作できない。"
        }
    }
}
