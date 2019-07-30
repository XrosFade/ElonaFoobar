config {
    core {
        screen {
            # Fullscreen mode.
            fullscreen = "windowed"

            music = true
            sound = true

            # Render shadows at higher quality.
            high_quality_shadows = true # shadow

            # Enable shadows on objects.
            object_shadows = true

            # Play heartbeat sound when health is low.
            heartbeat = true

            # Skip displaying random event popup windows.
            # The random events will still occur.
            skip_random_event_popups = false
        }

        anime {
            # Enable scrolling animations.
            scroll = true

            # Show scrolling animations when running.
            # Turning this off can speed up running somewhat.
            scroll_when_run = true

            # Always center the screen.
            always_center = true

            # Number of frames to wait for most animations/actions.
            # Valid values are 0 - 50.
            general_wait = 30 # wait1

            # Number of frames to wait for animations.
            # This also acts as multiplier for the speed of auto-turn actions.
            # Valid values are 0 - 20.
            anime_wait = 20

            # Number of frames to wait if an important message is received.
            # These are: leveling up, leveling up a skill, or having a change in hunger status.
            # Valid values are 0 - 50.
            alert_wait = 50

            # Speed of auto-turn actions.
            # O2 # hide, left, rightne of: "normal", "high", "highest"
            auto_turn_speed = "normal"

            # Play animations when melee attacking.
            attack_anime = true

            # Play weather-related animations.
            weather_effect = true

            # Play animations at the title screen.
            title_effect = true

            # Play window animations.
            window_anime = false

            # Number of frames to wait between updates to animations in the screen, like rain/lighting.
            # This does not affect key delay or animations that block input.
            # Valid values are 0 - 15.
            screen_refresh = 2
        }

        net {
            enabled = false
            wish = false
            chat = false
            server_list = false
        }

        font {
            # TTF fonts to use.
            # Place fonts in data/font. Please ensure the fonts are monospaced to avoid issues.
            file = ""

            vertical_offset = -1 # fontVfix1
            size_adjustment = 1 # fontSfix1

            # Font quality. "low" or "high" is available.
            quality = "high"
        }

        game {
            # Place the name of a save folder to load on startup here, like "sav_noa".
            default_save = ""

            # Attack non-hostile, non-ally NPCs when running into them.
            attack_neutral_npcs = false # ignore_dislike

            # Enable playback of the story.
            story = true

            # Show extra help popups for new players.
            extra_help = true

            # Hide identify status updates from Sense Quality.
            hide_autoidentify = false

            # Hide daily shop reports of items sold for player-owned shops.
            hide_shop_updates = false # hide_shopResult
        }

        debug {
            wizard = false
        }

        message {
            add_timestamps = false

            # Message box transparency.
            # Valid values are 0 - 10.
            transparency = 4
        }

        input {
            joypad = false
            autodisable_numlock = true

            # Number of frames to wait between movement commands when walking.
            # Valid values are 0 - 10.
            walk_wait = 5

            # Number of movement commands to run when walking before starting to run.
            # Valid values are 0 - 5.
            start_run_wait = 2

            # Number of frames to wait between movement commands when running.
            # Valid values are 0 - 10.
            run_wait = 2

            # Number of frames to wait between consecutive attacks when running into enemies.
            # Valid values are 0 - 10.
            attack_wait = 4

            # Number of frames to wait between presses of shortcut keys.
            # Valid values are 1 - 10.
            key_wait = 5
        }

        # These settings affect game balance.
        balance {
            # Interval in days it takes shops to restock items.
            # If 0, the item selection will change each time you interact with a shopkeeper.
            # Valid values are 0 - 10.
            restock_interval = 3

            # Enable extra races in character creation.
            extra_race = true

            # Enable extra classes in character creation.
            extra_class = true
        }

        # Extra config settings added in Elona foobar.
        foobar {
            # Position to display the HP bar.
            # One of: "hide", "left", "right"
            hp_bar_position = "right"

            # Shows an icon for pets that are leashed.
            leash_icon = true

            # Automatically pick up items you pass over.
            autopick = true

            # Automatically save the game at certain intervals.
            autosave = false

            # Run a script in the data/script/ folder at startup.
            # Provide a script's name, like "my_script.lua" for "data/script/my_script.lua".
            startup_script = ""

            # Set to true to run startup scripts in a loaded save.
            # This may be dangerous, so back up your save data first.
            run_script_in_save = false

            # Show damage popups.
            damage_popup = true

            # Skip confirm to buy or sell items at town shop.
            skip_confirm_at_shop = false

            # Skip confirm to over-cast spells.
            skip_overcasting_warning = false
        }
    }
}
