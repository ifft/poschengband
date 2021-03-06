/* File: object2.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies. Other copyrights may also apply.
 */

/* Purpose: Object code, part 2 */

#include "angband.h"

#include "int-map.h"
#include "kajitips.h"

#include <assert.h>

#define ACTIVATION_CHANCE (p_ptr->prace == RACE_MON_RING ? 2 : 5)

/*
 * Excise a dungeon object from any stacks
 */
void excise_object_idx(int o_idx)
{
    object_type *j_ptr;

    s16b this_o_idx, next_o_idx = 0;

    s16b prev_o_idx = 0;


    /* Object */
    j_ptr = &o_list[o_idx];

    /* Monster */
    if (j_ptr->held_m_idx)
    {
        monster_type *m_ptr;

        /* Monster */
        m_ptr = &m_list[j_ptr->held_m_idx];

        /* Scan all objects in the grid */
        for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
        {
            object_type *o_ptr;

            /* Acquire object */
            o_ptr = &o_list[this_o_idx];

            /* Acquire next object */
            next_o_idx = o_ptr->next_o_idx;

            /* Done */
            if (this_o_idx == o_idx)
            {
                /* No previous */
                if (prev_o_idx == 0)
                {
                    /* Remove from list */
                    m_ptr->hold_o_idx = next_o_idx;
                }

                /* Real previous */
                else
                {
                    object_type *k_ptr;

                    /* Previous object */
                    k_ptr = &o_list[prev_o_idx];

                    /* Remove from list */
                    k_ptr->next_o_idx = next_o_idx;
                }

                /* Forget next pointer */
                o_ptr->next_o_idx = 0;

                /* Done */
                break;
            }

            /* Save prev_o_idx */
            prev_o_idx = this_o_idx;
        }
    }

    /* Dungeon */
    else
    {
        cave_type *c_ptr;

        int y = j_ptr->iy;
        int x = j_ptr->ix;

        /* Grid */
        c_ptr = &cave[y][x];

        /* Scan all objects in the grid */
        for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
        {
            object_type *o_ptr;

            /* Acquire object */
            o_ptr = &o_list[this_o_idx];

            /* Acquire next object */
            next_o_idx = o_ptr->next_o_idx;

            /* Done */
            if (this_o_idx == o_idx)
            {
                /* No previous */
                if (prev_o_idx == 0)
                {
                    /* Remove from list */
                    c_ptr->o_idx = next_o_idx;
                }

                /* Real previous */
                else
                {
                    object_type *k_ptr;

                    /* Previous object */
                    k_ptr = &o_list[prev_o_idx];

                    /* Remove from list */
                    k_ptr->next_o_idx = next_o_idx;
                }

                /* Forget next pointer */
                o_ptr->next_o_idx = 0;

                /* Done */
                break;
            }

            /* Save prev_o_idx */
            prev_o_idx = this_o_idx;
        }
    }
    p_ptr->window |= PW_OBJECT_LIST;
}


/*
 * Delete a dungeon object
 *
 * Handle "stacks" of objects correctly.
 */
void delete_object_idx(int o_idx)
{
    object_type *j_ptr;

    /* Excise */
    excise_object_idx(o_idx);

    /* Object */
    j_ptr = &o_list[o_idx];

    /* Dungeon floor */
    if (!(j_ptr->held_m_idx))
    {
        int y, x;

        /* Location */
        y = j_ptr->iy;
        x = j_ptr->ix;

        /* Visual update */
        lite_spot(y, x);
    }

    /* Wipe the object */
    object_wipe(j_ptr);

    /* Count objects */
    o_cnt--;

    p_ptr->window |= PW_OBJECT_LIST;
}


/*
 * Deletes all objects at given location
 */
void delete_object(int y, int x)
{
    cave_type *c_ptr;

    s16b this_o_idx, next_o_idx = 0;


    /* Refuse "illegal" locations */
    if (!in_bounds(y, x)) return;


    /* Grid */
    c_ptr = &cave[y][x];

    /* Scan all objects in the grid */
    for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
    {
        object_type *o_ptr;

        /* Acquire object */
        o_ptr = &o_list[this_o_idx];

        /* Acquire next object */
        next_o_idx = o_ptr->next_o_idx;

        /* Wipe the object */
        object_wipe(o_ptr);

        /* Count objects */
        o_cnt--;
    }

    /* Objects are gone */
    c_ptr->o_idx = 0;

    /* Visual update */
    lite_spot(y, x);

    p_ptr->window |= PW_OBJECT_LIST;
}


/*
 * Move an object from index i1 to index i2 in the object list
 */
static void compact_objects_aux(int i1, int i2)
{
    int i;

    cave_type *c_ptr;

    object_type *o_ptr;


    /* Do nothing */
    if (i1 == i2) return;


    /* Repair objects */
    for (i = 1; i < o_max; i++)
    {
        /* Acquire object */
        o_ptr = &o_list[i];

        /* Skip "dead" objects */
        if (!o_ptr->k_idx) continue;

        /* Repair "next" pointers */
        if (o_ptr->next_o_idx == i1)
        {
            /* Repair */
            o_ptr->next_o_idx = i2;
        }
    }


    /* Acquire object */
    o_ptr = &o_list[i1];


    /* Monster */
    if (o_ptr->held_m_idx)
    {
        monster_type *m_ptr;

        /* Acquire monster */
        m_ptr = &m_list[o_ptr->held_m_idx];

        /* Repair monster */
        if (m_ptr->hold_o_idx == i1)
        {
            /* Repair */
            m_ptr->hold_o_idx = i2;
        }
    }

    /* Dungeon */
    else
    {
        int y, x;

        /* Acquire location */
        y = o_ptr->iy;
        x = o_ptr->ix;

        /* Acquire grid */
        c_ptr = &cave[y][x];

        /* Repair grid */
        if (c_ptr->o_idx == i1)
        {
            /* Repair */
            c_ptr->o_idx = i2;
        }
    }


    /* Structure copy */
    o_list[i2] = o_list[i1];

    /* Wipe the hole */
    object_wipe(o_ptr);
}


/*
 * Compact and Reorder the object list
 *
 * This function can be very dangerous, use with caution!
 *
 * When actually "compacting" objects, we base the saving throw on a
 * combination of object level, distance from player, and current
 * "desperation".
 *
 * After "compacting" (if needed), we "reorder" the objects into a more
 * compact order, and we reset the allocation info, and the "live" array.
 */
void compact_objects(int size)
{
    int i, y, x, num, cnt;
    int cur_lev, cur_dis, chance;
    object_type *o_ptr;


    /* Compact */
    if (size)
    {
        /* Message */
        msg_print("Compacting objects...");


        /* Redraw map */
        p_ptr->redraw |= (PR_MAP);

        /* Window stuff */
        p_ptr->window |= (PW_OVERHEAD | PW_DUNGEON);
    }


    /* Compact at least 'size' objects */
    for (num = 0, cnt = 1; num < size; cnt++)
    {
        /* Get more vicious each iteration */
        cur_lev = 5 * cnt;

        /* Get closer each iteration */
        cur_dis = 5 * (20 - cnt);

        /* Examine the objects */
        for (i = 1; i < o_max; i++)
        {
            o_ptr = &o_list[i];

            /* Skip dead objects */
            if (!o_ptr->k_idx) continue;

            /* Hack -- High level objects start out "immune" */
            if (k_info[o_ptr->k_idx].level > cur_lev) continue;

            /* Monster */
            if (o_ptr->held_m_idx)
            {
                monster_type *m_ptr;

                /* Acquire monster */
                m_ptr = &m_list[o_ptr->held_m_idx];

                /* Get the location */
                y = m_ptr->fy;
                x = m_ptr->fx;

                /* Monsters protect their objects */
                if (randint0(100) < 90) continue;
            }

            /* Dungeon */
            else
            {
                /* Get the location */
                y = o_ptr->iy;
                x = o_ptr->ix;
            }

            /* Nearby objects start out "immune" */
            if ((cur_dis > 0) && (distance(py, px, y, x) < cur_dis)) continue;

            /* Saving throw */
            chance = 90;

            /* Hack -- only compact artifacts in emergencies */
            if ((object_is_fixed_artifact(o_ptr) || o_ptr->art_name) &&
                (cnt < 1000)) chance = 100;

            /* Apply the saving throw */
            if (randint0(100) < chance) continue;

            /* Delete the object */
            delete_object_idx(i);

            /* Count it */
            num++;
        }
    }


    /* Excise dead objects (backwards!) */
    for (i = o_max - 1; i >= 1; i--)
    {
        o_ptr = &o_list[i];

        /* Skip real objects */
        if (o_ptr->k_idx) continue;

        /* Move last object into open hole */
        compact_objects_aux(o_max - 1, i);

        /* Compress "o_max" */
        o_max--;
    }
}


/*
 * Delete all the items when player leaves the level
 *
 * Note -- we do NOT visually reflect these (irrelevant) changes
 *
 * Hack -- we clear the "c_ptr->o_idx" field for every grid,
 * and the "m_ptr->next_o_idx" field for every monster, since
 * we know we are clearing every object. Technically, we only
 * clear those fields for grids/monsters containing objects,
 * and we clear it once for every such object.
 */
void wipe_o_list(void)
{
    int i;

    /* Delete the existing objects */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr = &o_list[i];

        /* Skip dead objects */
        if (!o_ptr->k_idx) continue;

        /* Mega-Hack -- preserve artifacts */
        if (!character_dungeon || preserve_mode)
        {
            /* Hack -- Preserve unknown artifacts */
            if (object_is_fixed_artifact(o_ptr) && !object_is_known(o_ptr))
            {
                /* Mega-Hack -- Preserve the artifact */
                a_info[o_ptr->name1].cur_num = 0;
            }
            if (o_ptr->name3 && !object_is_known(o_ptr))
            {
                /* Mega-Hack -- Preserve the artifact */
                a_info[o_ptr->name3].cur_num = 0;
            }
        }

        /* Monster */
        if (o_ptr->held_m_idx)
        {
            monster_type *m_ptr;

            /* Monster */
            m_ptr = &m_list[o_ptr->held_m_idx];

            /* Hack -- see above */
            m_ptr->hold_o_idx = 0;
        }

        /* Dungeon */
        else
        {
            cave_type *c_ptr;

            /* Access location */
            int y = o_ptr->iy;
            int x = o_ptr->ix;

            /* Access grid */
            c_ptr = &cave[y][x];

            /* Hack -- see above */
            c_ptr->o_idx = 0;
        }

        /* Wipe the object */
        object_wipe(o_ptr);
    }

    /* Reset "o_max" */
    o_max = 1;

    /* Reset "o_cnt" */
    o_cnt = 0;
}


/*
 * Acquires and returns the index of a "free" object.
 *
 * This routine should almost never fail, but in case it does,
 * we must be sure to handle "failure" of this routine.
 */
s16b o_pop(void)
{
    int i;


    /* Initial allocation */
    if (o_max < max_o_idx)
    {
        /* Get next space */
        i = o_max;

        /* Expand object array */
        o_max++;

        /* Count objects */
        o_cnt++;

        /* Use this object */
        return (i);
    }


    /* Recycle dead objects */
    for (i = 1; i < o_max; i++)
    {
        object_type *o_ptr;

        /* Acquire object */
        o_ptr = &o_list[i];

        /* Skip live objects */
        if (o_ptr->k_idx) continue;

        /* Count objects */
        o_cnt++;

        /* Use this object */
        return (i);
    }


    /* Warn the player (except during dungeon creation) */
    if (character_dungeon) msg_print("Too many objects!");


    /* Oops */
    return (0);
}


/*
 * Apply a "object restriction function" to the "object allocation table"
 */
errr get_obj_num_prep(void)
{
    int i;

    /* Get the entry */
    alloc_entry *table = alloc_kind_table;

    /* Scan the allocation table */
    for (i = 0; i < alloc_kind_size; i++)
    {
        /* Accept objects which pass the restriction, if any */
        if (!get_obj_num_hook || (*get_obj_num_hook)(table[i].index))
        {
            /* Accept this object */
            table[i].prob2 = table[i].prob1;
        }

        /* Do not use this object */
        else
        {
            /* Decline this object */
            table[i].prob2 = 0;
        }
    }

    /* Success */
    return (0);
}


/*
 * Choose an object kind that seems "appropriate" to the given level
 *
 * This function uses the "prob2" field of the "object allocation table",
 * and various local information, to calculate the "prob3" field of the
 * same table, which is then used to choose an "appropriate" object, in
 * a relatively efficient manner.
 *
 * Note that if no objects are "appropriate", then this function will
 * fail, and return zero, but this should *almost* never happen.
 */
s16b get_obj_num(int level)
{
    int             i;
    int             k_idx;
    long            value, total;
    object_kind     *k_ptr;
    alloc_entry     *table = alloc_kind_table;

    if (level > MAX_DEPTH - 1) level = MAX_DEPTH - 1;

    /* Boost level */
    if ((level > 0) && !(d_info[dungeon_type].flags1 & DF1_BEGINNER))
    {
        /* Occasional "boost" */
        if (one_in_(GREAT_OBJ))
        {
            int boost = level;
            if (boost < 20)
                boost = 20;
            level += rand_range(boost/4, boost/2);

            /* What a bizarre calculation
            level = 1 + (level * MAX_DEPTH / randint1(MAX_DEPTH)); */
        }
    }

    /* Reset total */
    total = 0L;

    /* Process probabilities */
    for (i = 0; i < alloc_kind_size; i++)
    {
        /* Objects are sorted by depth */
        if (table[i].level > level) break;
        table[i].prob3 = 0;

        if (table[i].max_level && table[i].max_level < level) continue;

        k_idx = table[i].index;
        k_ptr = &k_info[k_idx];
        if (k_ptr->tval == TV_FOOD && k_ptr->sval == SV_FOOD_AMBROSIA && dungeon_type != DUNGEON_OLYMPUS) continue;

        /* Hack -- prevent embedded chests */
        if (opening_chest && (k_ptr->tval == TV_CHEST)) continue;

        table[i].prob3 = table[i].prob2;
        total += table[i].prob3;
    }

    /* No legal objects */
    if (total <= 0)
        return 0;


    /* Pick an object */
    value = randint0(total);

    /* Find the object */
    for (i = 0; i < alloc_kind_size; i++)
    {
        /* Found the entry */
        if (value < table[i].prob3) break;

        /* Decrement */
        value = value - table[i].prob3;
    }

    /* Note: There used to be power boosting code here, but it gave very bad results in
     * some situations. For example, I want wands, rods and staves to allocate equally, but
     * would like wands to be available earlier than staves, which are earlier than rods.
     * Setting things up as:
     *   Wand:  A:1/1
     *   Staff: A:5/1
     *   Rod:   A:10/1
     * gave the following allocation distribution (10,000 objects, devices are 15%, deeper than DL10
     * so I would expect 5% allocation to each):
     * > Wands:  286 2.8%
     * > Staves: 486 4.8%
     * > Rods:   747 7.4%
     * Adding duplicate allocation entries helped, but the distribution was still unacceptably
     * skewed and this "feature" seemed more like a bug to me. -CTK
     */

    /* Result */
    return (table[i].index);
}

bool object_is_aware(object_type *o_ptr)
{
    return k_info[o_ptr->k_idx].aware;
}

/*
 * Known is true when the "attributes" of an object are "known".
 * These include tohit, todam, toac, cost, and pval (charges).
 *
 * Note that "knowing" an object gives you everything that an "awareness"
 * gives you, and much more. In fact, the player is always "aware" of any
 * item of which he has full "knowledge".
 *
 * But having full knowledge of, say, one "wand of wonder", does not, by
 * itself, give you knowledge, or even awareness, of other "wands of wonder".
 * It happens that most "identify" routines (including "buying from a shop")
 * will make the player "aware" of the object as well as fully "know" it.
 *
 * This routine also removes any inscriptions generated by "feelings".
 */
void object_known(object_type *o_ptr)
{
    /* Remove "default inscriptions" */
    o_ptr->feeling = FEEL_NONE;

    /* Clear the "Felt" info */
    o_ptr->ident &= ~(IDENT_SENSE);

    /* Clear the "Empty" info */
    o_ptr->ident &= ~(IDENT_EMPTY);
    o_ptr->ident &= ~(IDENT_TRIED);

    /* Now we know about the item */
    o_ptr->ident |= (IDENT_KNOWN);
}

/*
 * The player is now aware of the effects of the given object.
 */
void object_aware(object_type *o_ptr)
{
    k_info[o_ptr->k_idx].aware = TRUE;
}
void ego_aware(object_type *o_ptr)
{
    if (o_ptr->name2)
        e_info[o_ptr->name2].aware = TRUE;
}
bool ego_is_aware(int which)
{
    if (e_info[which].aware || (e_info[which].gen_flags & TRG_AWARE))
        return TRUE;
    return FALSE;
}

/* Statistics
   We try hard not to leak information. For example, when picking up an
   unaware potion, we should wait for one of the following before counting it:
   [1] Identify
   [2] Sell to shop
   [3] Quaff and become aware.

   Note we might miss some counts if the user quaffs, but doesn't notice the
   effect. This is better than leaking kind info in the various browser screens, 
   though.
*/
counts_t stats_rand_art_counts = {0};

void stats_reset(void)
{
    int i;

    for (i = 1; i < max_k_idx; i++)
    {
        object_kind *k_ptr = &k_info[i];

        WIPE(&k_ptr->counts, counts_t);
    }
    for (i = 1; i < max_e_idx; i++)
    {
        ego_item_type *e_ptr = &e_info[i];

        WIPE(&e_ptr->counts, counts_t);
    }

    WIPE(&stats_rand_art_counts, counts_t);
    device_stats_reset();
}

void stats_on_load(savefile_ptr file)
{
    stats_rand_art_counts.generated = savefile_read_s32b(file);
    stats_rand_art_counts.found = savefile_read_s32b(file);
    stats_rand_art_counts.bought = savefile_read_s32b(file);
    stats_rand_art_counts.used = savefile_read_s32b(file);
    stats_rand_art_counts.destroyed = savefile_read_s32b(file);

    device_stats_on_load(file);
}

void stats_on_save(savefile_ptr file)
{
    savefile_write_s32b(file, stats_rand_art_counts.generated);
    savefile_write_s32b(file, stats_rand_art_counts.found);
    savefile_write_s32b(file, stats_rand_art_counts.bought);
    savefile_write_s32b(file, stats_rand_art_counts.used);      /* Artifact Devices */
    savefile_write_s32b(file, stats_rand_art_counts.destroyed); /* Certain Class Powers */

    device_stats_on_save(file);
}

void stats_on_purchase(object_type *o_ptr)
{
    if (!(o_ptr->marked & OM_COUNTED))
    {
        k_info[o_ptr->k_idx].counts.bought += o_ptr->number;
        o_ptr->marked |= OM_COUNTED;
    }
    if (object_is_device(o_ptr) && !(o_ptr->marked & OM_EFFECT_COUNTED))
    {
        device_stats_on_purchase(o_ptr);
        o_ptr->marked |= OM_EFFECT_COUNTED;
    }
    if (o_ptr->name2 && !(o_ptr->marked & OM_EGO_COUNTED))
    {
        e_info[o_ptr->name2].counts.bought += o_ptr->number;
        o_ptr->marked |= OM_EGO_COUNTED;
    }
    if (o_ptr->art_name && !(o_ptr->marked & OM_ART_COUNTED))
    {
        stats_rand_art_counts.bought += o_ptr->number;
        o_ptr->marked |= OM_ART_COUNTED;
    }
}

void stats_on_sell(object_type *o_ptr)
{
    if (!(o_ptr->marked & OM_COUNTED))
    {
        k_info[o_ptr->k_idx].counts.found += o_ptr->number;
        o_ptr->marked |= OM_COUNTED;
    }
    if (object_is_device(o_ptr) && !(o_ptr->marked & OM_EFFECT_COUNTED))
    {
        device_stats_on_find(o_ptr);
        o_ptr->marked |= OM_EFFECT_COUNTED;
    }
    if (o_ptr->name2 && !(o_ptr->marked & OM_EGO_COUNTED))
    {
        e_info[o_ptr->name2].counts.found += o_ptr->number;
        o_ptr->marked |= OM_EGO_COUNTED;
    }
    if (o_ptr->art_name && !(o_ptr->marked & OM_ART_COUNTED))
    {
        stats_rand_art_counts.found += o_ptr->number;
        o_ptr->marked |= OM_ART_COUNTED;
    }
}

void stats_on_notice(object_type *o_ptr, int num)
{
    if (!(o_ptr->marked & OM_COUNTED))
    {
        k_info[o_ptr->k_idx].counts.found += num;
        o_ptr->marked |= OM_COUNTED;
    }

    /* Note: Noticing the effect of a device now identifies the device */

    if (o_ptr->name2 && !(o_ptr->marked & OM_EGO_COUNTED))
    {
        e_info[o_ptr->name2].counts.found += num;
        o_ptr->marked |= OM_EGO_COUNTED;
    }
    if (o_ptr->art_name && !(o_ptr->marked & OM_ART_COUNTED))
    {
        stats_rand_art_counts.found += num;
        o_ptr->marked |= OM_ART_COUNTED;
    }
}

void stats_on_combine(object_type *dest, object_type *src)
{
    if (object_is_aware(dest) && !(dest->marked & OM_COUNTED))
    {
        k_info[dest->k_idx].counts.found += dest->number;
        dest->marked |= OM_COUNTED;
    }
    if (object_is_aware(src) && !(src->marked & OM_COUNTED))
    {
        k_info[src->k_idx].counts.found += src->number;
        src->marked |= OM_COUNTED;
    }

    /* Note: Devices no longer stack */
}

void stats_on_use(object_type *o_ptr, int num)
{
    k_info[o_ptr->k_idx].counts.used += num;
    if (o_ptr->name2)
        e_info[o_ptr->name2].counts.used += num;
    if (o_ptr->art_name)
        stats_rand_art_counts.used += num;

    if (object_is_device(o_ptr))
        device_stats_on_use(o_ptr, num);
}

void stats_on_p_destroy(object_type *o_ptr, int num)
{
    if (object_is_aware(o_ptr) && !(o_ptr->marked & OM_COUNTED))
    {
        k_info[o_ptr->k_idx].counts.found += o_ptr->number;
        o_ptr->marked |= OM_COUNTED;
    }
    if (object_is_device(o_ptr) && !(o_ptr->marked & OM_EFFECT_COUNTED))
    {
        device_stats_on_find(o_ptr);
        o_ptr->marked |= OM_EFFECT_COUNTED;
    }
    if (o_ptr->name2 && !(o_ptr->marked & OM_EGO_COUNTED))
    {
        e_info[o_ptr->name2].counts.found += o_ptr->number;
        o_ptr->marked |= OM_EGO_COUNTED;
    }
    if (o_ptr->art_name && !(o_ptr->marked & OM_ART_COUNTED))
    {
        stats_rand_art_counts.found += num;
        o_ptr->marked |= OM_ART_COUNTED;
    }

    k_info[o_ptr->k_idx].counts.destroyed += num;
    if (o_ptr->name2)
        e_info[o_ptr->name2].counts.destroyed += num;
    if (o_ptr->art_name)
        stats_rand_art_counts.destroyed += num;
    if (object_is_device(o_ptr))
        device_stats_on_destroy(o_ptr);
}

void stats_on_m_destroy(object_type *o_ptr, int num)
{
    k_info[o_ptr->k_idx].counts.destroyed += num;
    if (o_ptr->name2)
        e_info[o_ptr->name2].counts.destroyed += num;
    if (object_is_device(o_ptr))
        device_stats_on_destroy(o_ptr);
}

void stats_on_pickup(object_type *o_ptr)
{
    if (object_is_aware(o_ptr) && !(o_ptr->marked & OM_COUNTED))
    {
        k_info[o_ptr->k_idx].counts.found += o_ptr->number;
        o_ptr->marked |= OM_COUNTED;
    }
    if (object_is_known(o_ptr) && object_is_device(o_ptr) && !(o_ptr->marked & OM_EFFECT_COUNTED))
    {
        device_stats_on_find(o_ptr);
        o_ptr->marked |= OM_EFFECT_COUNTED;
    }
    if (object_is_known(o_ptr) && o_ptr->name2 && !(o_ptr->marked & OM_EGO_COUNTED))
    {
        e_info[o_ptr->name2].counts.found += o_ptr->number;
        o_ptr->marked |= OM_EGO_COUNTED;
    }

    if (object_is_known(o_ptr) && o_ptr->art_name && !(o_ptr->marked & OM_ART_COUNTED))
    {
        stats_rand_art_counts.found += o_ptr->number;
        o_ptr->marked |= OM_ART_COUNTED;
    }
}

void stats_on_equip(object_type *o_ptr)
{
    if (object_is_aware(o_ptr) && !(o_ptr->marked & OM_COUNTED))
    {
        k_info[o_ptr->k_idx].counts.found += o_ptr->number;
        o_ptr->marked |= OM_COUNTED;
    }

    if (object_is_known(o_ptr) && o_ptr->name2 && !(o_ptr->marked & OM_EGO_COUNTED))
    {
        e_info[o_ptr->name2].counts.found += o_ptr->number;
        o_ptr->marked |= OM_EGO_COUNTED;
    }

    if (object_is_known(o_ptr) && o_ptr->art_name && !(o_ptr->marked & OM_ART_COUNTED))
    {
        stats_rand_art_counts.found += o_ptr->number;
        o_ptr->marked |= OM_ART_COUNTED;
    }
}

void stats_on_identify(object_type *o_ptr)
{
    if (!(o_ptr->marked & OM_COUNTED))
    {
        k_info[o_ptr->k_idx].counts.found += o_ptr->number;
        o_ptr->marked |= OM_COUNTED;
    }

    if (object_is_device(o_ptr) && !(o_ptr->marked & OM_EFFECT_COUNTED))
    {
        device_stats_on_find(o_ptr);
        o_ptr->marked |= OM_EFFECT_COUNTED;
    }

    if (o_ptr->name2 && !(o_ptr->marked & OM_EGO_COUNTED))
    {
        e_info[o_ptr->name2].counts.found += o_ptr->number;
        o_ptr->marked |= OM_EGO_COUNTED;
    }

    if (o_ptr->art_name && !(o_ptr->marked & OM_ART_COUNTED))
    {
        stats_rand_art_counts.found += o_ptr->number;
        o_ptr->marked |= OM_ART_COUNTED;
    }
}

/*
 * Something has been "sampled"
 */
void object_tried(object_type *o_ptr)
{
    if (object_is_device(o_ptr))
        o_ptr->ident |= IDENT_TRIED;
    else
        k_info[o_ptr->k_idx].tried = TRUE;
}

bool object_is_tried(object_type *o_ptr)
{
    if (object_is_device(o_ptr))
        return (o_ptr->ident & IDENT_TRIED) ? TRUE : FALSE;
    else
        return k_info[o_ptr->k_idx].tried;
}

/*
 * Return the "value" of an "unknown" item
 * Make a guess at the value of non-aware items
 */
static s32b object_value_base(object_type *o_ptr)
{
    /* Aware item -- use template cost */
    if (object_is_aware(o_ptr)) return (k_info[o_ptr->k_idx].cost);

    /* Analyze the type */
    switch (o_ptr->tval)
    {

        /* Un-aware Food */
        case TV_FOOD: return (5L);

        /* Un-aware Potions */
        case TV_POTION: return (20L);

        /* Un-aware Scrolls */
        case TV_SCROLL: return (20L);

        /* Un-aware Staffs */
        case TV_STAFF: return (70L);

        /* Un-aware Wands */
        case TV_WAND: return (50L);

        /* Un-aware Rods */
        case TV_ROD: return (90L);

        /* Un-aware Rings */
        case TV_RING: return (45L);

        /* Un-aware Amulets */
        case TV_AMULET: return (45L);

        /* Figurines, relative to monster level */
        case TV_FIGURINE:
        {
            int level = r_info[o_ptr->pval].level;
            if (level < 20) return level*50L;
            else if (level < 30) return 1000+(level-20)*150L;
            else if (level < 40) return 2500+(level-30)*350L;
            else if (level < 50) return 6000+(level-40)*800L;
            else return 14000+(level-50)*2000L;
        }

        case TV_CAPTURE:
            if (!o_ptr->pval) return 1000L;
            else return ((r_info[o_ptr->pval].level) * 50L + 1000);
    }

    /* Paranoia -- Oops */
    return (0L);
}


/* Return the value of the flags the object has... */
s32b flag_cost(object_type *o_ptr, int plusses, bool hack)
{
    s32b total = 0;
    u32b flgs[TR_FLAG_SIZE];
    s32b tmp_cost;
    int count;
    int i;
    object_kind *k_ptr = &k_info[o_ptr->k_idx];

    object_flags(o_ptr, flgs);

    /*
     * Exclude fixed flags of the base item.
     * pval bonuses of base item will be treated later.
     */
    for (i = 0; i < TR_FLAG_SIZE; i++)
        flgs[i] &= ~(k_ptr->flags[i]);

    /* Exclude fixed flags of the fixed artifact. */
    if (object_is_fixed_artifact(o_ptr) && !hack)
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];

        for (i = 0; i < TR_FLAG_SIZE; i++)
            flgs[i] &= ~(a_ptr->flags[i]);
    }

    /* Exclude fixed flags of the ego-item. */
    else if (object_is_ego(o_ptr))
    {
        ego_item_type *e_ptr = &e_info[o_ptr->name2];

        for (i = 0; i < TR_FLAG_SIZE; i++)
            flgs[i] &= ~(e_ptr->flags[i]);
    }


    /*
     * Calucurate values of remaining flags
     */
    if (have_flag(flgs, TR_STR)) total += (1500 * plusses);
    if (have_flag(flgs, TR_INT)) total += (1500 * plusses);
    if (have_flag(flgs, TR_WIS)) total += (1500 * plusses);
    if (have_flag(flgs, TR_DEX)) total += (1500 * plusses);
    if (have_flag(flgs, TR_CON)) total += (1500 * plusses);
    if (have_flag(flgs, TR_CHR)) total += (750 * plusses);
    if (have_flag(flgs, TR_MAGIC_MASTERY)) total += (600 * plusses);
    if (have_flag(flgs, TR_STEALTH)) total += (250 * plusses);
    if (have_flag(flgs, TR_SEARCH)) total += (100 * plusses);
    if (have_flag(flgs, TR_INFRA)) total += (150 * plusses);
    if (have_flag(flgs, TR_TUNNEL)) total += (175 * plusses);
    if ((have_flag(flgs, TR_SPEED)) && (plusses > 0))
        total += (10000 * plusses);
    if ((have_flag(flgs, TR_BLOWS)) && (plusses > 0))
        total += (10000 * plusses);

    tmp_cost = 0;
    count = 0;
    if (have_flag(flgs, TR_CHAOTIC)) {total += 9000;count++;}
    if (have_flag(flgs, TR_ORDER)) {total += 12000;count++;}
    if (have_flag(flgs, TR_WILD)) {total += 20000;count++;}
    if (have_flag(flgs, TR_VAMPIRIC)) {total += 6500;count++;}
    if (have_flag(flgs, TR_FORCE_WEAPON)) {tmp_cost += 2500;count++;}
    if (have_flag(flgs, TR_KILL_ANIMAL)) {tmp_cost += 2800;count++;}
    else if (have_flag(flgs, TR_SLAY_ANIMAL)) {tmp_cost += 500;}
    if (have_flag(flgs, TR_KILL_EVIL)) {tmp_cost += 5800;count++;}
    else if (have_flag(flgs, TR_SLAY_EVIL)) {tmp_cost += 3800;count++;}
    if (have_flag(flgs, TR_KILL_HUMAN)) {tmp_cost += 2800;count++;}
    else if (have_flag(flgs, TR_SLAY_HUMAN)) {tmp_cost += 800;}
    if (have_flag(flgs, TR_KILL_UNDEAD)) {tmp_cost += 2800;count++;}
    else if (have_flag(flgs, TR_SLAY_UNDEAD)) {tmp_cost += 1000;}
    if (have_flag(flgs, TR_KILL_DEMON)) {tmp_cost += 2800;count++;}
    else if (have_flag(flgs, TR_SLAY_DEMON)) {tmp_cost += 800;}
    if (have_flag(flgs, TR_KILL_ORC)) {tmp_cost += 500;}
    else if (have_flag(flgs, TR_SLAY_ORC)) {tmp_cost += 150;}
    if (have_flag(flgs, TR_KILL_TROLL)) {tmp_cost += 280;}
    else if (have_flag(flgs, TR_SLAY_TROLL)) {tmp_cost += 180;}
    if (have_flag(flgs, TR_KILL_GIANT)) {tmp_cost += 1000;count++;}
    else if (have_flag(flgs, TR_SLAY_GIANT)) {tmp_cost += 180;}
    if (have_flag(flgs, TR_KILL_DRAGON)) {tmp_cost += 2800;count++;}
    else if (have_flag(flgs, TR_SLAY_DRAGON)) {tmp_cost += 1800;count++;}

    if (have_flag(flgs, TR_VORPAL)) {tmp_cost += 1500;count++;}
    if (have_flag(flgs, TR_VORPAL2)) {tmp_cost += 5000;count++;}
    if (have_flag(flgs, TR_IMPACT)) {tmp_cost += 500;}
    if (have_flag(flgs, TR_BRAND_POIS)) {tmp_cost += 1000;count++;}
    if (have_flag(flgs, TR_BRAND_ACID)) {tmp_cost += 1200;count++;}
    if (have_flag(flgs, TR_BRAND_ELEC)) {tmp_cost += 1000;count++;}
    if (have_flag(flgs, TR_BRAND_FIRE)) {tmp_cost += 1000;count++;}
    if (have_flag(flgs, TR_BRAND_COLD)) {tmp_cost += 1000;count++;}
    total += (tmp_cost * count);

    if (have_flag(flgs, TR_SUST_STR)) total += 250;
    if (have_flag(flgs, TR_SUST_INT)) total += 250;
    if (have_flag(flgs, TR_SUST_WIS)) total += 250;
    if (have_flag(flgs, TR_SUST_DEX)) total += 250;
    if (have_flag(flgs, TR_SUST_CON)) total += 250;
    if (have_flag(flgs, TR_SUST_CHR)) total += 50;
    if (have_flag(flgs, TR_RIDING)) total += 0;
    if (have_flag(flgs, TR_EASY_SPELL)) total += 1500;
    if (have_flag(flgs, TR_THROW)) total += 50;
    if (have_flag(flgs, TR_FREE_ACT)) total += 2500;
    if (have_flag(flgs, TR_HOLD_LIFE)) total += 1500;

    tmp_cost = 0;
    count = 0;
    if (have_flag(flgs, TR_IM_ACID)) {tmp_cost += 15000;count += 2;}
    if (have_flag(flgs, TR_IM_ELEC)) {tmp_cost += 15000;count += 2;}
    if (have_flag(flgs, TR_IM_FIRE)) {tmp_cost += 15000;count += 2;}
    if (have_flag(flgs, TR_IM_COLD)) {tmp_cost += 15000;count += 2;}
    if (have_flag(flgs, TR_REFLECT)) {tmp_cost += 5000;count += 2;}
    if (have_flag(flgs, TR_RES_ACID)) {tmp_cost += 500;count++;}
    if (have_flag(flgs, TR_RES_ELEC)) {tmp_cost += 500;count++;}
    if (have_flag(flgs, TR_RES_FIRE)) {tmp_cost += 500;count++;}
    if (have_flag(flgs, TR_RES_COLD)) {tmp_cost += 500;count++;}
    if (have_flag(flgs, TR_RES_POIS)) {tmp_cost += 1000;count += 1;}
    if (have_flag(flgs, TR_RES_FEAR)) {tmp_cost += 1000;count += 1;}
    if (have_flag(flgs, TR_RES_LITE)) {tmp_cost += 800;count += 1;}
    if (have_flag(flgs, TR_RES_DARK)) {tmp_cost += 800;count += 1;}
    if (have_flag(flgs, TR_RES_BLIND)) {tmp_cost += 900;count += 1;}
    if (have_flag(flgs, TR_RES_CONF)) {tmp_cost += 900;count += 1;}
    if (have_flag(flgs, TR_RES_SOUND)) {tmp_cost += 900;count += 1;}
    if (have_flag(flgs, TR_RES_SHARDS)) {tmp_cost += 900;count += 1;}
    if (have_flag(flgs, TR_RES_NETHER)) {tmp_cost += 900;count += 1;}
    if (have_flag(flgs, TR_RES_NEXUS)) {tmp_cost += 900;count += 1;}
    if (have_flag(flgs, TR_RES_CHAOS)) {tmp_cost += 1000;count += 1;}
    if (have_flag(flgs, TR_RES_DISEN)) {tmp_cost += 2000;count += 1;}
    total += (tmp_cost * count);

    if (have_flag(flgs, TR_SH_FIRE)) total += 100;
    if (have_flag(flgs, TR_SH_ELEC)) total += 100;
    if (have_flag(flgs, TR_SH_COLD)) total += 100;
    if (have_flag(flgs, TR_NO_TELE)) total -= 50000;
    if (have_flag(flgs, TR_NO_MAGIC)) total -= 2500;
    if (have_flag(flgs, TR_TY_CURSE)) total -= 15000;
    if (have_flag(flgs, TR_HIDE_TYPE)) total += 0;
    if (have_flag(flgs, TR_SHOW_MODS)) total += 0;
    if (have_flag(flgs, TR_LEVITATION)) total += 350;
    if (have_flag(flgs, TR_LITE)) total += 350;
    if (have_flag(flgs, TR_SEE_INVIS)) total += 500;
    if (have_flag(flgs, TR_TELEPATHY)) total += 20000;
    if (have_flag(flgs, TR_ESP_ANIMAL)) total += 1000;
    if (have_flag(flgs, TR_ESP_UNDEAD)) total += 1000;
    if (have_flag(flgs, TR_ESP_DEMON)) total += 1000;
    if (have_flag(flgs, TR_ESP_ORC)) total += 1000;
    if (have_flag(flgs, TR_ESP_TROLL)) total += 1000;
    if (have_flag(flgs, TR_ESP_GIANT)) total += 1000;
    if (have_flag(flgs, TR_ESP_DRAGON)) total += 1000;
    if (have_flag(flgs, TR_ESP_HUMAN)) total += 1000;
    if (have_flag(flgs, TR_ESP_EVIL)) total += 15000;
    if (have_flag(flgs, TR_ESP_GOOD)) total += 2000;
    if (have_flag(flgs, TR_ESP_NONLIVING)) total += 2000;
    if (have_flag(flgs, TR_ESP_UNIQUE)) total += 10000;
    if (have_flag(flgs, TR_SLOW_DIGEST)) total += 750;
    if (have_flag(flgs, TR_REGEN)) total += 2500;
    if (have_flag(flgs, TR_WARNING)) total += 2000;
    if (have_flag(flgs, TR_DEC_MANA)) total += 10000;
    if (have_flag(flgs, TR_XTRA_MIGHT)) total += 2250;
    if (have_flag(flgs, TR_XTRA_SHOTS)) total += 10000 * o_ptr->pval;
    if (have_flag(flgs, TR_IGNORE_ACID)) total += 100;
    if (have_flag(flgs, TR_IGNORE_ELEC)) total += 100;
    if (have_flag(flgs, TR_IGNORE_FIRE)) total += 100;
    if (have_flag(flgs, TR_IGNORE_COLD)) total += 100;
    if (have_flag(flgs, TR_DRAIN_EXP)) total -= 12500;
    if (have_flag(flgs, TR_TELEPORT))
    {
        if (object_is_cursed(o_ptr))
            total -= 7500;
        else
            total += 250;
    }
    if (have_flag(flgs, TR_AGGRAVATE)) total -= 10000;
    if (have_flag(flgs, TR_BLESSED)) total += 750;
    if (o_ptr->curse_flags & TRC_CURSED) total -= 5000;
    if (o_ptr->curse_flags & TRC_HEAVY_CURSE) total -= 12500;
    if (o_ptr->curse_flags & TRC_PERMA_CURSE) total -= 15000;

    /* Also, give some extra for activatable powers... */
    if (obj_has_effect(o_ptr))
    {
        effect_t effect = obj_get_effect(o_ptr);
        assert(effect.type);
        total += effect_value(&effect);
    }

    return total;
}


/*
 * Return the "real" price of a "known" item, not including discounts
 *
 */
s32b object_value_real(object_type *o_ptr)
{
    s32b value;

    u32b flgs[TR_FLAG_SIZE];

    object_kind *k_ptr = &k_info[o_ptr->k_idx];

    /* Dave has been kind enough to come up with much better scoring.
       So use the new algorithms whenever possible.
    */
    if (object_is_melee_weapon(o_ptr)) return weapon_cost(o_ptr, COST_REAL);
    if (o_ptr->tval == TV_BOW) return bow_cost(o_ptr, COST_REAL);
    if (object_is_armour(o_ptr) || object_is_shield(o_ptr)) return armor_cost(o_ptr, COST_REAL);
    if (object_is_jewelry(o_ptr) || (o_ptr->tval == TV_LITE && object_is_artifact(o_ptr))) return jewelry_cost(o_ptr, COST_REAL);
    if (o_ptr->tval == TV_LITE) return lite_cost(o_ptr, COST_REAL);
    if (object_is_device(o_ptr)) return device_value(o_ptr, COST_REAL);

    /* OK, here's the old pricing algorithm :( 
       Note this algorithm cheats for artifacts by relying on cost
       data from a_info.txt. The result was that rand-arts get scored
       poorly. Also, try comparing the code calculated cost to
       the human one in a_info.txt some time. The new algorithms
       are much nicer. */

    /* Hack -- "worthless" items */
    if (!k_info[o_ptr->k_idx].cost) return (0L);

    /* Base cost */
    value = k_info[o_ptr->k_idx].cost;

    /* Extract some flags */
    object_flags(o_ptr, flgs);

    /* Artifact */
    if (object_is_fixed_artifact(o_ptr))
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];

        /* Hack -- "worthless" artifacts */
        if (!a_ptr->cost) return (0L);

        /* Hack -- Use the artifact cost instead */
        value = a_ptr->cost;
        value += flag_cost(o_ptr, o_ptr->pval, FALSE);

        /* Don't add pval bonuses etc. */
        return (value);
    }

    /* Ego-Item */
    else if (object_is_ego(o_ptr))
    {
        /* Hack -- Reward the ego-item with a bonus */
        value += flag_cost(o_ptr, o_ptr->pval, FALSE);
    }

    else
    {
        int i;
        bool flag = FALSE;

        for (i = 0; i < TR_FLAG_SIZE; i++) 
            if (o_ptr->art_flags[i]) flag = TRUE;

        if (flag) value += flag_cost(o_ptr, o_ptr->pval, FALSE);
    }

    /* Analyze pval bonus for normal object */
    switch (o_ptr->tval)
    {
    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
    case TV_BOW:
    case TV_DIGGING:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_CROWN:
    case TV_SHIELD:
    case TV_CLOAK:
    case TV_SOFT_ARMOR:
    case TV_HARD_ARMOR:
    case TV_DRAG_ARMOR:
    case TV_LITE:
    case TV_AMULET:
    case TV_RING:
        /* No pval */
        if (!o_ptr->pval) break;

        /* Hack -- Negative "pval" is always bad */
        if (o_ptr->pval < 0) return (0L);

        /* Give credit for stat bonuses */
        if (have_flag(flgs, TR_STR)) value += (o_ptr->pval * 200L);
        if (have_flag(flgs, TR_INT)) value += (o_ptr->pval * 200L);
        if (have_flag(flgs, TR_WIS)) value += (o_ptr->pval * 200L);
        if (have_flag(flgs, TR_DEX)) value += (o_ptr->pval * 200L);
        if (have_flag(flgs, TR_CON)) value += (o_ptr->pval * 200L);
        if (have_flag(flgs, TR_CHR)) value += (o_ptr->pval * 200L);

        /* Give credit for stealth and searching */
        if (have_flag(flgs, TR_MAGIC_MASTERY)) value += (o_ptr->pval * 100);
        if (have_flag(flgs, TR_STEALTH)) value += (o_ptr->pval * 100L);
        if (have_flag(flgs, TR_SEARCH)) value += (o_ptr->pval * 100L);

        /* Give credit for infra-vision and tunneling */
        if (have_flag(flgs, TR_INFRA)) value += (o_ptr->pval * 50L);
        if (have_flag(flgs, TR_TUNNEL)) value += (o_ptr->pval * 50L);

        /* Give credit for extra attacks */
        if (have_flag(flgs, TR_BLOWS)) value += (o_ptr->pval * 5000L);

        /* Give credit for speed bonus */
        if (have_flag(flgs, TR_SPEED)) value += (o_ptr->pval * 10000L);

        break;
    }


    /* Analyze the item */
    switch (o_ptr->tval)
    {
        /* Rings/Amulets */
        case TV_RING:
        case TV_AMULET:
        {
            /* Hack -- negative bonuses are bad */
            if (o_ptr->to_h + o_ptr->to_d + o_ptr->to_a < 0) return (0L);

            /* Give credit for bonuses */
            value += ((o_ptr->to_h + o_ptr->to_d + o_ptr->to_a) * 200L);

            /* Done */
            break;
        }

        /* Armor */
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_CLOAK:
        case TV_CROWN:
        case TV_HELM:
        case TV_SHIELD:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:
        {
            /* Hack -- negative armor bonus */
            if (o_ptr->to_a < 0) return (0L);

            /* Give credit for bonuses */
            value += (((o_ptr->to_h - k_ptr->to_h) + (o_ptr->to_d - k_ptr->to_d)) * 200L + (o_ptr->to_a) * 100L);

            /* Done */
            break;
        }

        /* Bows/Weapons */
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_SWORD:
        case TV_POLEARM:
        {
            /* Hack -- negative hit/damage bonuses */
            if (o_ptr->to_h + o_ptr->to_d < 0) return (0L);

            /* Factor in the bonuses */
            value += ((o_ptr->to_h + o_ptr->to_d + o_ptr->to_a) * 100L);

            /* Hack -- Factor in extra damage dice and sides */
            value += (o_ptr->dd - k_ptr->dd) * o_ptr->ds * 250L;
            value += (o_ptr->ds - k_ptr->ds) * o_ptr->dd * 250L;

            /* Done */
            break;
        }

        /* Ammo */
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        {
            /* Hack -- negative hit/damage bonuses */
            if (o_ptr->to_h + o_ptr->to_d < 0) return (0L);

            /* Factor in the bonuses */
            value += ((o_ptr->to_h + o_ptr->to_d) * 5L);

            /* Hack -- Factor in extra damage dice and sides */
            value += (o_ptr->dd - k_ptr->dd) * o_ptr->ds * 5L;
            value += (o_ptr->ds - k_ptr->ds) * o_ptr->dd * 5L;

            /* Done */
            break;
        }

        /* Figurines, relative to monster level */
        case TV_FIGURINE:
        {
            int level = r_info[o_ptr->pval].level;
            if (level < 20) value = level*50L;
            else if (level < 30) value = 1000+(level-20)*150L;
            else if (level < 40) value = 2500+(level-30)*350L;
            else if (level < 50) value = 6000+(level-40)*800L;
            else value = 14000+(level-50)*2000L;
            break;
        }

        case TV_CAPTURE:
        {
            if (!o_ptr->pval) value = 1000L;
            else value = ((r_info[o_ptr->pval].level) * 50L + 1000);
            break;
        }

        case TV_CHEST:
        {
            if (!o_ptr->pval) value = 0L;
            break;
        }
    }

    /* Worthless object */
    if (value < 0) return 0L;

    /* Return the value */
    return (value);
}


/*
 * Return the price of an item including plusses (and charges)
 *
 * This function returns the "value" of the given item (qty one)
 *
 * Never notice "unknown" bonuses or properties, including "curses",
 * since that would give the player information he did not have.
 *
 * Note that discounted items stay discounted forever, even if
 * the discount is "forgotten" by the player via memory loss.
 */
s32b object_value(object_type *o_ptr)
{
    s32b value;
    if (object_is_known(o_ptr))
    {
        if (object_is_melee_weapon(o_ptr))
            value = weapon_cost(o_ptr, 0);
        else if (o_ptr->tval == TV_BOW)
            value = bow_cost(o_ptr, 0);
        else if (object_is_armour(o_ptr) || object_is_shield(o_ptr))
            value = armor_cost(o_ptr, 0);
        else if (object_is_jewelry(o_ptr) || (o_ptr->tval == TV_LITE && object_is_artifact(o_ptr)))
            value = jewelry_cost(o_ptr, 0);
        else if (o_ptr->tval == TV_LITE)
            value = lite_cost(o_ptr, 0);
        else if (object_is_device(o_ptr))
            value = device_value(o_ptr, 0);
        else
            value = object_value_real(o_ptr);

        if (!(o_ptr->ident & IDENT_FULL))
        {
            if (o_ptr->name2 && !e_info[o_ptr->name2].aware)
                value += 500;
            else if (object_is_artifact(o_ptr))
                value += 1000;
        }
    }
    else
    {
        value = new_object_cost(o_ptr, 0);
        if (!value)
            value = object_value_base(o_ptr);
        if ((o_ptr->ident & IDENT_SENSE) && object_is_cursed(o_ptr))
            value /= 3;
        if ((o_ptr->ident & IDENT_SENSE) && object_is_ego(o_ptr))
            value += 500;
        if ((o_ptr->ident & IDENT_SENSE) && object_is_artifact(o_ptr))
            value += 1000;
    }
    if (o_ptr->discount) 
        value -= (value * o_ptr->discount / 100L);
    return value;
}


/*
 * Determines whether an object can be destroyed, and makes fake inscription.
 */
bool can_player_destroy_object(object_type *o_ptr)
{
    /* Artifacts cannot be destroyed */
    if (!object_is_artifact(o_ptr) || (o_ptr->rune == RUNE_SACRIFICE)) return TRUE;

    /* If object is unidentified, makes fake inscription */
    if (!object_is_known(o_ptr))
    {
        byte feel = FEEL_SPECIAL;

        /* Hack -- Handle icky artifacts */
        if (object_is_cursed(o_ptr) || object_is_broken(o_ptr)) feel = FEEL_TERRIBLE;

        /* Hack -- inscribe the artifact */
        o_ptr->feeling = feel;

        /* We have "felt" it (again) */
        o_ptr->ident |= (IDENT_SENSE);

        /* Combine the pack */
        p_ptr->notice |= (PN_COMBINE);

        /* Window stuff */
        p_ptr->window |= (PW_INVEN | PW_EQUIP);

        /* Done */
        return FALSE;
    }

    /* Identified artifact -- Nothing to do */
    return FALSE;
}


/*
 * Distribute charges of rods or wands.
 *
 * o_ptr = source item
 * q_ptr = target item, must be of the same type as o_ptr
 * amt   = number of items that are transfered
 */
void distribute_charges(object_type *o_ptr, object_type *q_ptr, int amt)
{
    /*
     * Hack -- If rods or wands are dropped, the total maximum timeout or
     * charges need to be allocated between the two stacks. If all the items
     * are being dropped, it makes for a neater message to leave the original
     * stack's pval alone. -LM-
     */
    if ((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_ROD))
    {
        q_ptr->pval = o_ptr->pval * amt / o_ptr->number;
        if (amt < o_ptr->number) o_ptr->pval -= q_ptr->pval;

        /* Hack -- Rods also need to have their timeouts distributed. The
         * dropped stack will accept all time remaining to charge up to its
         * maximum.
         */
        if ((o_ptr->tval == TV_ROD) && (o_ptr->timeout))
        {
            if (q_ptr->pval > o_ptr->timeout)
                q_ptr->timeout = o_ptr->timeout;
            else
                q_ptr->timeout = q_ptr->pval;

            if (amt < o_ptr->number) o_ptr->timeout -= q_ptr->timeout;
        }
    }
}

void reduce_charges(object_type *o_ptr, int amt)
{
    /*
     * Hack -- If rods or wand are destroyed, the total maximum timeout or
     * charges of the stack needs to be reduced, unless all the items are
     * being destroyed. -LM-
     */
    if (((o_ptr->tval == TV_WAND) || (o_ptr->tval == TV_ROD)) &&
        (amt < o_ptr->number))
    {
        o_ptr->pval -= o_ptr->pval * amt / o_ptr->number;
    }
}


/*
 * Determine if an item can "absorb" a second item
 *
 * See "object_absorb()" for the actual "absorption" code.
 *
 * If permitted, we allow staffs (if they are known to have equal charges
 * and both are either known or confirmed empty) and wands (if both are
 * either known or confirmed empty) and rods (in all cases) to combine.
 * Staffs will unstack (if necessary) when they are used, but wands and
 * rods will only unstack if one is dropped. -LM-
 *
 * If permitted, we allow weapons/armor to stack, if fully "known".
 *
 * Missiles will combine if both stacks have the same "known" status.
 * This is done to make unidentified stacks of missiles useful.
 *
 * Food, potions, scrolls, and "easy know" items always stack.
 *
 * Chests, and activatable items, never stack (for various reasons).
 */

/*
 * A "stack" of items is limited to less than or equal to 99 items (hard-coded).
 */
#define MAX_STACK_SIZE 99


/*
 *  Determine if an item can partly absorb a second item.
 *  Return maximum number of stack.
 */
int object_similar_part(object_type *o_ptr, object_type *j_ptr)
{
    int i;

    /* Default maximum number of stack */
    int max_num = MAX_STACK_SIZE;

    /* Require identical object types */
    if (o_ptr->k_idx != j_ptr->k_idx) return 0;


    /* Analyze the items */
    switch (o_ptr->tval)
    {
        /* Chests and Statues*/
        case TV_CHEST:
        case TV_CARD:
        case TV_CAPTURE:
        case TV_RUNE:
        {
            /* Never okay */
            return 0;
        }

        case TV_STATUE:
        {
            if ((o_ptr->sval != SV_PHOTO) || (j_ptr->sval != SV_PHOTO)) return 0;
            if (o_ptr->pval != j_ptr->pval) return 0;
            break;
        }

        /* Figurines and Corpses*/
        case TV_FIGURINE:
        case TV_CORPSE:
        {
            /* Same monster */
            if (o_ptr->pval != j_ptr->pval) return 0;

            /* Assume okay */
            break;
        }

        /* Food and Potions and Scrolls */
        case TV_FOOD:
        case TV_POTION:
        case TV_SCROLL:
        {
            if (o_ptr->art_name || j_ptr->art_name)
                return 0;
            /* Assume okay */
            break;
        }

        /* Staffs */
        case TV_STAFF:
        case TV_WAND:
        case TV_ROD:
            return 0;

        /* Weapons and Armor */
        case TV_BOW:
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_SWORD:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_SOFT_ARMOR:
        case TV_HARD_ARMOR:
        case TV_DRAG_ARMOR:

        /* Rings, Amulets, Lites */
        case TV_RING:
        case TV_AMULET:
        case TV_LITE:
        case TV_WHISTLE:
        {
            /* Require full knowledge of both items */
            if (!object_is_known(o_ptr) || !object_is_known(j_ptr)) return 0;

            /* Fall through */
        }

        /* Missiles */
        case TV_BOLT:
        case TV_ARROW:
        case TV_SHOT:
        {
            /* Require identical knowledge of both items */
            if (object_is_known(o_ptr) != object_is_known(j_ptr)) return 0;
            if (o_ptr->feeling != j_ptr->feeling) return 0;

            /* Require identical "bonuses" */
            if (o_ptr->to_h != j_ptr->to_h) return 0;
            if (o_ptr->to_d != j_ptr->to_d) return 0;
            if (o_ptr->to_a != j_ptr->to_a) return 0;

            /* Require identical "pval" code */
            if (o_ptr->pval != j_ptr->pval) return 0;

            /* Artifacts never stack */
            if (object_is_artifact(o_ptr) || object_is_artifact(j_ptr)) return 0;

            /* Require identical "ego-item" names */
            if (o_ptr->name2 != j_ptr->name2) return 0;

            /* Require identical added essence  */
            if (o_ptr->xtra3 != j_ptr->xtra3) return 0;
            if (o_ptr->xtra4 != j_ptr->xtra4) return 0;

            /* Hack -- Never stack "powerful" items */
            if (o_ptr->xtra1 || j_ptr->xtra1) return 0;

            /* Hack -- Never stack recharging items */
            if (o_ptr->timeout || j_ptr->timeout) return 0;

            /* Require identical "values" */
            if (o_ptr->ac != j_ptr->ac) return 0;
            if (o_ptr->dd != j_ptr->dd) return 0;
            if (o_ptr->ds != j_ptr->ds) return 0;

            /* Probably okay */
            break;
        }

        /* Various */
        default:
        {
            /* Require knowledge */
            if (!object_is_known(o_ptr) || !object_is_known(j_ptr)) return 0;

            /* Probably okay */
            break;
        }
    }


    /* Hack -- Identical art_flags! */
    for (i = 0; i < TR_FLAG_SIZE; i++)
        if (o_ptr->art_flags[i] != j_ptr->art_flags[i]) return 0;

    /* Hack -- Require identical "cursed" status */
    if (o_ptr->curse_flags != j_ptr->curse_flags) return 0;

    /* Require identical activations */
    if ( o_ptr->activation.type != j_ptr->activation.type
      || o_ptr->activation.cost != j_ptr->activation.cost
      || o_ptr->activation.power != j_ptr->activation.power
      || o_ptr->activation.difficulty != j_ptr->activation.difficulty
      || o_ptr->activation.extra != j_ptr->activation.extra )
    {
        return 0;
    }

    /* Hack -- Require identical "broken" status */
    if ((o_ptr->ident & (IDENT_BROKEN)) != (j_ptr->ident & (IDENT_BROKEN))) return 0;


    /* Hack -- require semi-matching "inscriptions" */
    if (o_ptr->inscription && j_ptr->inscription &&
        (o_ptr->inscription != j_ptr->inscription))
        return 0;

    /* Hack -- normally require matching "inscriptions" */
    if (!stack_force_notes && (o_ptr->inscription != j_ptr->inscription)) return 0;

    /* Hack -- normally require matching "discounts" */
    if (!stack_force_costs && (o_ptr->discount != j_ptr->discount)) return 0;


    /* They match, so they must be similar */
    return max_num;
}

/*
 *  Determine if an item can absorb a second item.
 */
bool object_similar(object_type *o_ptr, object_type *j_ptr)
{
    int total = o_ptr->number + j_ptr->number;
    int max_num;

    /* Are these objects similar? */
    max_num = object_similar_part(o_ptr, j_ptr);

    /* Return if not similar */
    if (!max_num) return FALSE;

    /* Maximal "stacking" limit */
    if (total > max_num) return (0);


    /* They match, so they must be similar */
    return (TRUE);
}



/*
 * Allow one item to "absorb" another, assuming they are similar
 */
void object_absorb(object_type *o_ptr, object_type *j_ptr)
{
    int max_num = object_similar_part(o_ptr, j_ptr);
    int total = o_ptr->number + j_ptr->number;
    int diff = (total > max_num) ? total - max_num : 0;

    /* Combine quantity, lose excess items */
    o_ptr->number = (total > max_num) ? max_num : total;

    /* Hack -- blend "known" status */
    if (object_is_known(j_ptr)) object_known(o_ptr);

    /* Hack -- clear "storebought" if only one has it */
    if (((o_ptr->ident & IDENT_STORE) || (j_ptr->ident & IDENT_STORE)) &&
        (!((o_ptr->ident & IDENT_STORE) && (j_ptr->ident & IDENT_STORE))))
    {
        if (j_ptr->ident & IDENT_STORE) j_ptr->ident &= 0xEF;
        if (o_ptr->ident & IDENT_STORE) o_ptr->ident &= 0xEF;
    }

    /* Hack -- blend "mental" status */
    if (j_ptr->ident & (IDENT_FULL)) o_ptr->ident |= (IDENT_FULL);

    /* Hack -- blend "inscriptions" */
    if (j_ptr->inscription) o_ptr->inscription = j_ptr->inscription;

    /* Hack -- blend "feelings" */
    if (j_ptr->feeling) o_ptr->feeling = j_ptr->feeling;

    /* Hack -- could average discounts XXX XXX XXX */
    /* Hack -- save largest discount XXX XXX XXX */
    if (o_ptr->discount < j_ptr->discount) o_ptr->discount = j_ptr->discount;

    /* Hack -- if rods are stacking, add the pvals (maximum timeouts) and current timeouts together. -LM- */
    if (o_ptr->tval == TV_ROD)
    {
        o_ptr->pval += j_ptr->pval * (j_ptr->number - diff) / j_ptr->number;
        o_ptr->timeout += j_ptr->timeout * (j_ptr->number - diff) / j_ptr->number;
    }

    /* Hack -- if wands are stacking, combine the charges. -LM- */
    if (o_ptr->tval == TV_WAND)
    {
        o_ptr->pval += j_ptr->pval * (j_ptr->number - diff) / j_ptr->number;
    }
}


/*
 * Find the index of the object_kind with the given tval and sval
 */
s16b lookup_kind(int tval, int sval)
{
    int k;
    int num = 0;
    int bk = 0;

    /* Look for it */
    for (k = 1; k < max_k_idx; k++)
    {
        object_kind *k_ptr = &k_info[k];

        /* Require correct tval */
        if (k_ptr->tval != tval) continue;

        /* Found a match */
        if (k_ptr->sval == sval) return (k);

        /* Ignore illegal items */
        if (sval != SV_ANY) continue;

        /* Apply the randomizer */
        ++num;
        if (!one_in_(num)) continue; /* beware the evil of macros! */

        /* Use this value */
        bk = k;
    }

    /* Return this choice */
    if (sval == SV_ANY)
    {
        return bk;
    }

    /* Oops */
    return (0);
}


/*
 * Wipe an object clean.
 */
void object_wipe(object_type *o_ptr)
{
    /* Wipe the structure */
    (void)WIPE(o_ptr, object_type); 
}


/*
 * Prepare an object based on an existing object
 */
void object_copy(object_type *o_ptr, object_type *j_ptr)
{
    /* Copy the structure */
    COPY(o_ptr, j_ptr, object_type);
}


/*
 * Prepare an object based on an object kind.
 */
void object_prep(object_type *o_ptr, int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    /* Clear the record */
    object_wipe(o_ptr);

    /* Save the kind index */
    o_ptr->k_idx = k_idx;
    if (k_ptr->tval != TV_GOLD && !store_hack)
    {
        k_ptr->counts.generated++;
    }

    /* Efficiency -- tval/sval */
    o_ptr->tval = k_ptr->tval;
    o_ptr->sval = k_ptr->sval;

    /* Default "pval" */
    o_ptr->pval = k_ptr->pval;

    /* Default number */
    o_ptr->number = 1;

    /* Default weight */
    o_ptr->weight = k_ptr->weight;

    /* Default magic */
    o_ptr->to_h = k_ptr->to_h;
    o_ptr->to_d = k_ptr->to_d;
    o_ptr->to_a = k_ptr->to_a;

    /* Default power */
    o_ptr->ac = k_ptr->ac;
    o_ptr->dd = k_ptr->dd;
    o_ptr->ds = k_ptr->ds;
    o_ptr->mult = k_ptr->mult;

    /* Hack -- worthless items are always "broken" */
    if (k_info[o_ptr->k_idx].cost <= 0) o_ptr->ident |= (IDENT_BROKEN);

    /* Hack -- cursed items are always "cursed" */
    if (k_ptr->gen_flags & (TRG_CURSED)) o_ptr->curse_flags |= (TRC_CURSED);
    if (k_ptr->gen_flags & (TRG_HEAVY_CURSE)) o_ptr->curse_flags |= (TRC_HEAVY_CURSE);
    if (k_ptr->gen_flags & (TRG_PERMA_CURSE)) o_ptr->curse_flags |= (TRC_PERMA_CURSE);
    if (k_ptr->gen_flags & (TRG_RANDOM_CURSE0)) o_ptr->curse_flags |= get_curse(0, o_ptr);
    if (k_ptr->gen_flags & (TRG_RANDOM_CURSE1)) o_ptr->curse_flags |= get_curse(1, o_ptr);
    if (k_ptr->gen_flags & (TRG_RANDOM_CURSE2)) o_ptr->curse_flags |= get_curse(2, o_ptr);
}


/*
 * Help determine an "enchantment bonus" for an object.
 *
 * To avoid floating point but still provide a smooth distribution of bonuses,
 * we simply round the results of division in such a way as to "average" the
 * correct floating point value.
 *
 * This function has been changed. It uses "randnor()" to choose values from
 * a normal distribution, whose mean moves from zero towards the max as the
 * level increases, and whose standard deviation is equal to 1/4 of the max,
 * and whose values are forced to lie between zero and the max, inclusive.
 *
 * Since the "level" rarely passes 100 before Morgoth is dead, it is very
 * rare to get the "full" enchantment on an object, even a deep levels.
 *
 * It is always possible (albeit unlikely) to get the "full" enchantment.
 *
 * A sample distribution of values from "m_bonus(10, N)" is shown below:
 *
 *   N       0     1     2     3     4     5     6     7     8     9    10
 * ---    ----  ----  ----  ----  ----  ----  ----  ----  ----  ----  ----
 *   0   66.37 13.01  9.73  5.47  2.89  1.31  0.72  0.26  0.12  0.09  0.03
 *   8   46.85 24.66 12.13  8.13  4.20  2.30  1.05  0.36  0.19  0.08  0.05
 *  16   30.12 27.62 18.52 10.52  6.34  3.52  1.95  0.90  0.31  0.15  0.05
 *  24   22.44 15.62 30.14 12.92  8.55  5.30  2.39  1.63  0.62  0.28  0.11
 *  32   16.23 11.43 23.01 22.31 11.19  7.18  4.46  2.13  1.20  0.45  0.41
 *  40   10.76  8.91 12.80 29.51 16.00  9.69  5.90  3.43  1.47  0.88  0.65
 *  48    7.28  6.81 10.51 18.27 27.57 11.76  7.85  4.99  2.80  1.22  0.94
 *  56    4.41  4.73  8.52 11.96 24.94 19.78 11.06  7.18  3.68  1.96  1.78
 *  64    2.81  3.07  5.65  9.17 13.01 31.57 13.70  9.30  6.04  3.04  2.64
 *  72    1.87  1.99  3.68  7.15 10.56 20.24 25.78 12.17  7.52  4.42  4.62
 *  80    1.02  1.23  2.78  4.75  8.37 12.04 27.61 18.07 10.28  6.52  7.33
 *  88    0.70  0.57  1.56  3.12  6.34 10.06 15.76 30.46 12.58  8.47 10.38
 *  96    0.27  0.60  1.25  2.28  4.30  7.60 10.77 22.52 22.51 11.37 16.53
 * 104    0.22  0.42  0.77  1.36  2.62  5.33  8.93 13.05 29.54 15.23 22.53
 * 112    0.15  0.20  0.56  0.87  2.00  3.83  6.86 10.06 17.89 27.31 30.27
 * 120    0.03  0.11  0.31  0.46  1.31  2.48  4.60  7.78 11.67 25.53 45.72
 * 128    0.02  0.01  0.13  0.33  0.83  1.41  3.24  6.17  9.57 14.22 64.07
 */
s16b m_bonus(int max, int level)
{
    int bonus, stand, extra, value;


    /* Paranoia -- enforce maximal "level" */
    if (level > MAX_DEPTH - 1) level = MAX_DEPTH - 1;


    /* The "bonus" moves towards the max */
    bonus = ((max * level) / MAX_DEPTH);

    /* Hack -- determine fraction of error */
    extra = ((max * level) % MAX_DEPTH);

    /* Hack -- simulate floating point computations */
    if (randint0(MAX_DEPTH) < extra) bonus++;


    /* The "stand" is equal to one quarter of the max */
    stand = (max / 4);

    /* Hack -- determine fraction of error */
    extra = (max % 4);

    /* Hack -- simulate floating point computations */
    if (randint0(4) < extra) stand++;


    /* Choose an "interesting" value */
    value = randnor(bonus, stand);

    /* Enforce the minimum value */
    if (value < 0) return (0);

    /* Enforce the maximum value */
    if (value > max) return (max);

    /* Result */
    return (value);
}


/*
 * Cheat -- describe a created object for the user
 */
static void object_mention(object_type *o_ptr)
{
    char o_name[MAX_NLEN];

    /* Describe */
    object_desc(o_name, o_ptr, (OD_NAME_ONLY | OD_STORE));

    /* Artifact */
    if (object_is_fixed_artifact(o_ptr))
    {
        /* Silly message */
        msg_format("Artifact (%s)", o_name);

    }

    /* Random Artifact */
    else if (o_ptr->art_name)
    {
        msg_print("Random artifact");

    }

    /* Ego-item */
    else if (object_is_ego(o_ptr))
    {
        /* Silly message */
        msg_format("Ego-item (%s)", o_name);

    }

    /* Normal item */
    else
    {
        /* Silly message */
        msg_format("Object (%s)", o_name);

    }
}

static void dragon_resist(object_type * o_ptr)
{
    do
    {
        if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_DRAGON_FANG && one_in_(3))
            one_ele_slay(o_ptr);
        else if (one_in_(4))
            one_dragon_ele_resistance(o_ptr);
        else
            one_high_resistance(o_ptr);
    }
    while (one_in_(2));
}

/*
 * Mega-Hack -- Attempt to create one of the "Special Objects"
 *
 * We are only called from "make_object()", and we assume that
 * "apply_magic()" is called immediately after we return.
 *
 * Note -- see "make_artifact()" and "apply_magic()"
 */
static bool make_artifact_special(object_type *o_ptr)
{
    int i;
    int k_idx = 0;


    /* No artifacts in the town */
    if (!dun_level) return (FALSE);
    if (no_artifacts) return FALSE;

    /* Themed object */
    if (get_obj_num_hook) return (FALSE);

    /* Check the artifact list (just the "specials") */
    for (i = 0; i < max_a_idx; i++)
    {
        artifact_type *a_ptr = &a_info[i];

        if (!a_ptr->name) continue;
        if (a_ptr->cur_num) continue;
        if (a_ptr->gen_flags & TRG_QUESTITEM) continue;
        if (!(a_ptr->gen_flags & TRG_INSTA_ART)) continue;

        /* XXX XXX Enforce minimum "depth" (loosely) */
        if (a_ptr->level > dun_level)
        {
            /* Acquire the "out-of-depth factor" */
            int d = (a_ptr->level - dun_level) * 2;

            /* Roll for out-of-depth creation */
            if (!one_in_(d)) continue;
        }

        /* Artifact "rarity roll" */
        if (!one_in_(a_ptr->rarity)) continue;

        /* Find the base object */
        k_idx = lookup_kind(a_ptr->tval, a_ptr->sval);

        /* XXX XXX Enforce minimum "object" level (loosely) */
        if (k_info[k_idx].level > object_level)
        {
            /* Acquire the "out-of-depth factor" */
            int d = (k_info[k_idx].level - object_level) * 5;

            /* Roll for out-of-depth creation */
            if (!one_in_(d)) continue;
        }

        if (random_artifacts)
        {
            create_replacement_art(i, o_ptr);
        }
        else
        {
            create_named_art_aux(i, o_ptr);
        }
        /* Success */
        return (TRUE);
    }

    /* Failure */
    return (FALSE);
}


/*
 * Attempt to change an object into an artifact
 *
 * This routine should only be called by "apply_magic()"
 *
 * Note -- see "make_artifact_special()" and "apply_magic()"
 */
static bool make_artifact(object_type *o_ptr)
{
    int i;


    /* No artifacts in the town */
    if (!dun_level) return FALSE;
    if (no_artifacts) return FALSE;

    /* Paranoia -- no "plural" artifacts */
    if (o_ptr->number != 1) return (FALSE);

    /* Check the artifact list (skip the "specials") */
    for (i = 0; i < max_a_idx; i++)
    {
        artifact_type *a_ptr = &a_info[i];

        if (!a_ptr->name) continue;
        if (a_ptr->cur_num) continue;
        if (a_ptr->gen_flags & TRG_QUESTITEM) continue;
        if (a_ptr->gen_flags & TRG_INSTA_ART) continue;
        if (a_ptr->tval != o_ptr->tval) continue;
        if (a_ptr->sval != o_ptr->sval) continue;

        /* XXX XXX Enforce minimum "depth" (loosely) */
        if (a_ptr->level > dun_level)
        {
            /* Acquire the "out-of-depth factor" */
            int d = (a_ptr->level - dun_level) * 2;

            /* Roll for out-of-depth creation */
            if (!one_in_(d)) continue;
        }

        if (!one_in_(a_ptr->rarity)) continue;

        if (random_artifacts)
        {
            create_replacement_art(i, o_ptr);
        }
        else
        {
            create_named_art_aux(i, o_ptr);
        }
        /* Success */
        return (TRUE);
    }

    /* Failure */
    return (FALSE);
}


/*
 *  Choose random ego type
 */
int        apply_magic_ego = 0;
static int _get_random_ego(int type)
{
    int i, value;
    ego_item_type *e_ptr;

    int total = 0;

    if (apply_magic_ego)
        return apply_magic_ego;
    
    for (i = 1; i < max_e_idx; i++)
    {
        e_ptr = &e_info[i];
        
        if (e_ptr->type == type)
        {
            int rarity = e_ptr->rarity;
            if (rarity)
            {
                if (e_ptr->max_level && object_level > e_ptr->max_level)
                    rarity += 3*(object_level - e_ptr->max_level)/4;
                else if (e_ptr->level && object_level < e_ptr->level)
                    rarity += 3*rarity*(e_ptr->level - object_level)/4;
                total += MAX(10000 / rarity, 1);
            }
        }
    }

    value = randint1(total);

    for (i = 1; i < max_e_idx; i++)
    {
        e_ptr = &e_info[i];
        
        if (e_ptr->type == type)
        {
            int rarity = e_ptr->rarity;
            if (rarity)
            {
                if (e_ptr->max_level && object_level > e_ptr->max_level)
                    rarity += 3*(object_level - e_ptr->max_level)/4;
                else if (e_ptr->level && object_level < e_ptr->level)
                    rarity += 3*rarity*(e_ptr->level - object_level)/4;
                value -= MAX(10000 / rarity, 1);
                if (value <= 0) 
                    return i;
            }
        }
    }

    return 0;
}

static void _create_artifact(object_type *o_ptr, int power)
{
    u32b mode = CREATE_ART_NORMAL;
    
    if (power < 0)
        mode = CREATE_ART_CURSED;

    create_artifact(o_ptr, mode);
}

static bool _create_level_check(int power, int lvl)
{
    lvl = MAX(1, lvl);
    if (randint0(power * 100 / lvl) < 100)
        return TRUE;
    return FALSE;
}

static int _jewelry_pval(int max, int level)
{
    return randint1(1 + m_bonus(max - 1, level));
}

static int _jewelry_powers(int num, int level, int power)
{
    return abs(power) + m_bonus(num, level);
}

static void _finalize_jewelry(object_type *o_ptr)
{
    if (have_flag(o_ptr->art_flags, TR_RES_ACID))
        add_flag(o_ptr->art_flags, TR_IGNORE_ACID);
    if (have_flag(o_ptr->art_flags, TR_RES_ELEC))
        add_flag(o_ptr->art_flags, TR_IGNORE_ELEC);
    if (have_flag(o_ptr->art_flags, TR_RES_FIRE))
        add_flag(o_ptr->art_flags, TR_IGNORE_FIRE);
    if (have_flag(o_ptr->art_flags, TR_RES_COLD))
        add_flag(o_ptr->art_flags, TR_IGNORE_COLD);
}

static void _create_defender(object_type *o_ptr, int level, int power)
{
    add_flag(o_ptr->art_flags, TR_FREE_ACT);
    add_flag(o_ptr->art_flags, TR_SEE_INVIS);
    if (abs(power) >= 2 && level > 50)
    {
        if (one_in_(2))
            add_flag(o_ptr->art_flags, TR_LEVITATION);
        while (one_in_(2))
            one_sustain(o_ptr);
        o_ptr->to_a = 5 + randint1(7) + m_bonus(7, level);
        switch (randint1(4))
        {
        case 1: /* Classic Defender */
            add_flag(o_ptr->art_flags, TR_RES_ACID);
            add_flag(o_ptr->art_flags, TR_RES_ELEC);
            add_flag(o_ptr->art_flags, TR_RES_FIRE);
            add_flag(o_ptr->art_flags, TR_RES_COLD);
            if (one_in_(3))
                add_flag(o_ptr->art_flags, TR_RES_POIS);
            else
                one_high_resistance(o_ptr);
            break;
        case 2: /* High Defender */
            one_high_resistance(o_ptr);
            do
            {
                one_high_resistance(o_ptr);
            }
            while (one_in_(2));
            break;
        case 3: /* Lordly Protection */
            o_ptr->to_a += 5;
            add_flag(o_ptr->art_flags, TR_RES_POIS);
            add_flag(o_ptr->art_flags, TR_RES_DISEN);
            add_flag(o_ptr->art_flags, TR_HOLD_LIFE);
            do
            {
                one_lordly_high_resistance(o_ptr);
            }
            while (one_in_(4));
            break;
        case 4: /* Revenge! */
            add_flag(o_ptr->art_flags, TR_SH_COLD);
            add_flag(o_ptr->art_flags, TR_SH_ELEC);
            add_flag(o_ptr->art_flags, TR_SH_FIRE);
            if (one_in_(2))
                add_flag(o_ptr->art_flags, TR_SH_SHARDS);
            if (one_in_(7))
                add_flag(o_ptr->art_flags, TR_SH_REVENGE);
            break;
        }
    }
    else
    {
        if (one_in_(5))
            add_flag(o_ptr->art_flags, TR_LEVITATION);
        if (one_in_(5))
            one_sustain(o_ptr);
        o_ptr->to_a = randint1(5) + m_bonus(5, level);

        if (one_in_(3))
        {
            one_high_resistance(o_ptr);
            one_high_resistance(o_ptr);
        }
        else
        {
            one_ele_resistance(o_ptr);
            one_ele_resistance(o_ptr);
            one_ele_resistance(o_ptr);
            one_ele_resistance(o_ptr);
            one_ele_resistance(o_ptr);
            one_ele_resistance(o_ptr);
            one_ele_resistance(o_ptr);
        }
    }
    if (one_in_(ACTIVATION_CHANCE))
        effect_add_random(o_ptr, BIAS_PROTECTION);
}

static void _create_ring(object_type *o_ptr, int level, int power, int mode)
{
    int powers = 0;
    bool done = FALSE;
    bool force_great = FALSE;

    if (!apply_magic_ego && level > 50)
    {
        if ((mode & (AM_GOOD | AM_GREAT)) && randint0(150) < level)
        {
            force_great = TRUE;
        }
    }

    while (!done)
    {
        o_ptr->name2 = _get_random_ego(EGO_TYPE_RING);
        done = TRUE;
        if ( force_great
          && o_ptr->name2 != EGO_RING_SPEED
          && o_ptr->name2 != EGO_RING_DEFENDER )
        {
            done = FALSE;
        }
    }

    switch (o_ptr->name2)
    {
    case EGO_RING_DWARVES:
        o_ptr->to_d += 5;
        if (one_in_(3))
            add_flag(o_ptr->art_flags, TR_DEC_DEX);
        if (one_in_(3))
            add_flag(o_ptr->art_flags, TR_WIS);
        if (one_in_(6))
            o_ptr->curse_flags |= TRC_PERMA_CURSE;
        if (one_in_(6))
            add_flag(o_ptr->art_flags, TR_AGGRAVATE);
        if (one_in_(2))
            add_flag(o_ptr->art_flags, TR_RES_DARK);
        if (one_in_(3))
            add_flag(o_ptr->art_flags, TR_RES_DISEN);
        if (one_in_(3))
            add_flag(o_ptr->art_flags, TR_SUST_STR);
        if (one_in_(6))
            one_high_resistance(o_ptr);
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_PRIESTLY);
        break;
    case EGO_RING_NAZGUL:
        o_ptr->to_d += 6;
        o_ptr->to_h += 6;
        if (one_in_(6))
            o_ptr->curse_flags |= TRC_PERMA_CURSE;
        if (one_in_(66))
            add_flag(o_ptr->art_flags, TR_IM_COLD);
        if (one_in_(6))
            add_flag(o_ptr->art_flags, TR_SLAY_GOOD);
        if (one_in_(6))
            add_flag(o_ptr->art_flags, TR_BRAND_COLD);
        if (one_in_(6))
            add_flag(o_ptr->art_flags, TR_SLAY_HUMAN);
        if (one_in_(6))
            one_high_resistance(o_ptr);
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_NECROMANTIC);
        break;
    case EGO_RING_COMBAT:
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(7))
            {
            case 1:
                if (!have_flag(o_ptr->art_flags, TR_CON))
                {
                    add_flag(o_ptr->art_flags, TR_CON);
                    if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
                    break;
                }
            case 2:
                if (!have_flag(o_ptr->art_flags, TR_DEX))
                {
                    add_flag(o_ptr->art_flags, TR_DEX);
                    if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
                    break;
                }
            case 3:
                add_flag(o_ptr->art_flags, TR_STR);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
                break;
            case 4:
                o_ptr->to_h += randint1(5) + m_bonus(5, level);
                while (one_in_(2) && powers > 0)
                {
                    o_ptr->to_h += randint1(5) + m_bonus(5, level);
                    powers--;
                }
                break;
            case 5:
                o_ptr->to_d += randint1(5) + m_bonus(5, level);
                while (one_in_(2) && powers > 0)
                {
                    o_ptr->to_d += randint1(5) + m_bonus(5, level);
                    powers--;
                }
                break;
            case 6:
                if (abs(power) >= 2 && one_in_(30) && level >= 50)
                {
                    add_flag(o_ptr->art_flags, TR_WEAPONMASTERY);
                    o_ptr->pval = _jewelry_pval(3, level);
                    if (one_in_(30))
                    {
                        switch (randint1(5))
                        {
                        case 1: add_flag(o_ptr->art_flags, TR_BRAND_ACID); break;
                        case 2: add_flag(o_ptr->art_flags, TR_BRAND_COLD); break;
                        case 3: add_flag(o_ptr->art_flags, TR_BRAND_FIRE); break;
                        case 4: add_flag(o_ptr->art_flags, TR_BRAND_ELEC); break;
                        case 5: add_flag(o_ptr->art_flags, TR_BRAND_POIS); break;
                        }
                    }
                    break;
                }
                if (abs(power) >= 2 && one_in_(15) && level >= 50)
                {
                    add_flag(o_ptr->art_flags, TR_BLOWS);
                    o_ptr->pval = _jewelry_pval(3, level);
                    powers = 0;
                }
                else if (one_in_(3))
                {
                    add_flag(o_ptr->art_flags, TR_RES_FEAR);
                    break;
                }
            default:
                o_ptr->to_d += randint1(5) + m_bonus(5, level);
            }
        }
        if (o_ptr->to_h > 25) o_ptr->to_h = 25;
        if (o_ptr->to_d > 20) o_ptr->to_d = 20;
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_WARRIOR | BIAS_STR);
        break;
    case EGO_RING_ARCHERY:
        for (powers = _jewelry_powers(4, level, power); powers > 0; --powers)
        {
            switch (randint1(7))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_DEX);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
                break;
            case 2:
                add_flag(o_ptr->art_flags, TR_STEALTH);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
                break;
            case 3:
                o_ptr->to_h += randint1(5) + m_bonus(5, level);
                break;
            case 4:
                o_ptr->to_d += randint1(5) + m_bonus(5, level);
                break;
            case 5:
                if ( (abs(power) >= 2 || _create_level_check(200, level))
                  && (!have_flag(o_ptr->art_flags, TR_XTRA_MIGHT) || one_in_(7) ) )
                {
                    add_flag(o_ptr->art_flags, TR_XTRA_SHOTS);
                    o_ptr->pval = _jewelry_pval(5, level);
                    break;
                }
            case 6:
                if ( (abs(power) >= 2  || _create_level_check(200, level))
                  && (!have_flag(o_ptr->art_flags, TR_XTRA_SHOTS) || one_in_(7) ) )
                {
                    add_flag(o_ptr->art_flags, TR_XTRA_MIGHT);
                    o_ptr->pval = _jewelry_pval(5, level);
                    break;
                }
            default:
                o_ptr->to_h += randint1(5) + m_bonus(5, level);
            }
        }
        if (o_ptr->to_h > 25) o_ptr->to_h = 25;
        if (o_ptr->to_d > 20) o_ptr->to_d = 20;
        if ( o_ptr->pval > 3
          && (have_flag(o_ptr->art_flags, TR_XTRA_SHOTS) || have_flag(o_ptr->art_flags, TR_XTRA_MIGHT))
          && !one_in_(10) )
        {
            o_ptr->pval = 3;
        }
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_ARCHER);
        break;
    case EGO_RING_PROTECTION:
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(7))
            {
            case 1:
                one_high_resistance(o_ptr);
                if (abs(power) >= 2)
                {
                    do { one_high_resistance(o_ptr); --power; } while(one_in_(3));
                }
                break;
            case 2:
                if (!have_flag(o_ptr->art_flags, TR_FREE_ACT))
                {
                    add_flag(o_ptr->art_flags, TR_FREE_ACT);
                    break;
                }
            case 3:
                if (!have_flag(o_ptr->art_flags, TR_SEE_INVIS))
                {
                    add_flag(o_ptr->art_flags, TR_SEE_INVIS);
                    break;
                }
            case 4:
                if (one_in_(2))
                {
                    add_flag(o_ptr->art_flags, TR_WARNING);
                    if (one_in_(3))
                        one_low_esp(o_ptr);
                    break;
                }
            case 5:
                if (one_in_(2))
                {
                    one_sustain(o_ptr);
                    if (abs(power) >= 2)
                    {
                        do { one_sustain(o_ptr); --power; } while(one_in_(2));
                    }
                    break;
                }
            default:
                o_ptr->to_a += randint1(10);
            }
        }
        if (o_ptr->to_a > 35) o_ptr->to_a = 35;
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_PROTECTION);
        break;
    case EGO_RING_ELEMENTAL:
        if (abs(power) >= 2)
        {
            switch (randint1(6))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_RES_COLD);
                add_flag(o_ptr->art_flags, TR_RES_FIRE);
                add_flag(o_ptr->art_flags, TR_RES_ELEC);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_RES_ACID);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_RES_POIS);
                break;
            case 2:
                o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);
                add_flag(o_ptr->art_flags, TR_RES_FIRE);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_SH_FIRE);
                if (one_in_(7))
                    add_flag(o_ptr->art_flags, TR_BRAND_FIRE);
                else if (randint1(level) >= 70)
                    add_flag(o_ptr->art_flags, TR_IM_FIRE);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_FIRE);
                break;
            case 3:
                o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);
                add_flag(o_ptr->art_flags, TR_RES_COLD);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_SH_COLD);
                if (one_in_(7))
                    add_flag(o_ptr->art_flags, TR_BRAND_COLD);
                else if (randint1(level) >= 70)
                    add_flag(o_ptr->art_flags, TR_IM_COLD);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_COLD);
                break;
            case 4:
                o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);
                add_flag(o_ptr->art_flags, TR_RES_ELEC);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_SH_ELEC);
                if (one_in_(7))
                    add_flag(o_ptr->art_flags, TR_BRAND_ELEC);
                else if (randint1(level) >= 75)
                    add_flag(o_ptr->art_flags, TR_IM_ELEC);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_ELEC);
                break;
            case 5:
                o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);
                add_flag(o_ptr->art_flags, TR_RES_ACID);
                if (one_in_(7))
                    add_flag(o_ptr->art_flags, TR_BRAND_ACID);
                else if (randint1(level) >= 65)
                    add_flag(o_ptr->art_flags, TR_IM_ACID);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_ACID);
                break;
            case 6:
                o_ptr->to_a = 5 + randint1(5) + m_bonus(10, level);
                add_flag(o_ptr->art_flags, TR_RES_SHARDS);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_SH_SHARDS);
                break;
            }
        }
        else
        {
            one_ele_resistance(o_ptr);
            if (one_in_(3))
                one_ele_resistance(o_ptr);
            if (one_in_(ACTIVATION_CHANCE))
                effect_add_random(o_ptr, BIAS_ELEMENTAL);
        }
        break;
    case EGO_RING_DEFENDER:
        _create_defender(o_ptr, level, power);
        break;
    case EGO_RING_SPEED:
        o_ptr->pval = randint1(5) + m_bonus(5, level);
        while (randint0(100) < 50) 
            o_ptr->pval++;
        if (cheat_peek) object_mention(o_ptr);
        if (one_in_(ACTIVATION_CHANCE*2))
        {
            if (one_in_(777))
                effect_add(o_ptr, EFFECT_LIGHT_SPEED);
            else if (one_in_(77))
                effect_add(o_ptr, EFFECT_SPEED_HERO);
            else
                effect_add(o_ptr, EFFECT_SPEED);
        }
        break;
    case EGO_RING_WIZARDRY:
        for (powers = _jewelry_powers(4, level, power); powers > 0; --powers)
        {
            switch (randint1(7))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_INT);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
                break;
            case 2:
                add_flag(o_ptr->art_flags, TR_SUST_INT);
                break;
            case 3:
                add_flag(o_ptr->art_flags, TR_SPELL_CAP);
                if (!o_ptr->pval) 
                    o_ptr->pval = _jewelry_pval(3, level);
                else if (o_ptr->pval > 3)
                    o_ptr->pval = 3;
                break;
            case 4:
                add_flag(o_ptr->art_flags, TR_EASY_SPELL);
                break;
            case 5:
                if (abs(power) >= 2)
                {
                    add_flag(o_ptr->art_flags, TR_DEC_MANA);
                    break;
                }
                else
                {
                    add_flag(o_ptr->art_flags, TR_LEVITATION);
                    break;
                }
            case 6:
                if (abs(power) >= 2 && one_in_(30))
                {
                    add_flag(o_ptr->art_flags, TR_SPELL_POWER);
                    add_flag(o_ptr->art_flags, TR_DEC_STR);
                    add_flag(o_ptr->art_flags, TR_DEC_DEX);
                    add_flag(o_ptr->art_flags, TR_DEC_CON);
                    o_ptr->pval = _jewelry_pval(2, level);
                }
                else
                {
                    o_ptr->to_d += randint1(5) + m_bonus(5, level);
                    while (one_in_(2) && powers > 0)
                    {
                        o_ptr->to_d += randint1(5) + m_bonus(5, level);
                        powers--;
                    }
                }
                break;
            default:
                if (abs(power) >= 2 && one_in_(15))
                    add_flag(o_ptr->art_flags, TR_TELEPATHY);
                else
                    one_low_esp(o_ptr);
            }
        }
        if (o_ptr->to_d > 20)
            o_ptr->to_d = 20;
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_MAGE);
        break;
    }

    _finalize_jewelry(o_ptr);

    /* Be sure to cursify later! */
    if (power == -1)
        power--;
}

static void _create_amulet(object_type *o_ptr, int level, int power, int mode)
{
    int powers = 0;
    bool force_great = FALSE;
    bool done = FALSE;

    if (!apply_magic_ego && level > 50)
    {
        if ((mode & (AM_GOOD | AM_GREAT)) && randint0(300) < level)
        {
            force_great = TRUE;
        }
    }

    while (!done)
    {
        o_ptr->name2 = _get_random_ego(EGO_TYPE_AMULET);
        done = TRUE;
        if ( force_great
          && o_ptr->name2 != EGO_AMULET_DEFENDER
          && o_ptr->name2 != EGO_AMULET_HERO )
        {
            done = FALSE;
        }
    }

    switch (o_ptr->name2)
    {
    case EGO_AMULET_MAGI:
        add_flag(o_ptr->art_flags, TR_SEARCH);
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(7))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_FREE_ACT);
                add_flag(o_ptr->art_flags, TR_SEE_INVIS);
                break;
            case 2:
                add_flag(o_ptr->art_flags, TR_SUST_INT);
                break;
            case 3:
                add_flag(o_ptr->art_flags, TR_EASY_SPELL);
                break;
            case 4:
                if (abs(power) >= 2 && one_in_(2))
                    add_flag(o_ptr->art_flags, TR_TELEPATHY);
                else
                    one_low_esp(o_ptr);
                break;
            case 5:
                if (abs(power) >= 2)
                {
                    add_flag(o_ptr->art_flags, TR_DEC_MANA);
                    break;
                }
                else if (one_in_(2))
                {
                    add_flag(o_ptr->art_flags, TR_MAGIC_MASTERY);
                    if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(6, level);
                    break;
                }
            case 6:
                if (abs(power) >= 2 && one_in_(15))
                {
                    add_flag(o_ptr->art_flags, TR_SPELL_POWER);
                    add_flag(o_ptr->art_flags, TR_DEC_STR);
                    add_flag(o_ptr->art_flags, TR_DEC_DEX);
                    add_flag(o_ptr->art_flags, TR_DEC_CON);
                    o_ptr->pval = _jewelry_pval(2, level);
                }
                else
                {
                    o_ptr->to_d += randint1(5) + m_bonus(5, level);
                    while (one_in_(2) && powers > 0)
                    {
                        o_ptr->to_d += randint1(5) + m_bonus(5, level);
                        powers--;
                    }
                }
                break;
            default:
                add_flag(o_ptr->art_flags, TR_INT);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
            }
        }
        if (!o_ptr->pval) o_ptr->pval = randint1(8); /* Searching */
        if (o_ptr->to_d > 20)
            o_ptr->to_d = 20;
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_MAGE);
        break;
    case EGO_AMULET_DEVOTION:
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(7))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_CHR);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
                break;
            case 2:
                add_flag(o_ptr->art_flags, TR_REFLECT);
                break;
            case 3:
                if (abs(power) >= 2 && one_in_(2) && level >= 30)
                {
                    add_flag(o_ptr->art_flags, TR_SPELL_CAP);
                    o_ptr->pval = _jewelry_pval(3, level);
                }
                else
                {
                    add_flag(o_ptr->art_flags, TR_HOLD_LIFE);
                    if (one_in_(2))
                        add_flag(o_ptr->art_flags, TR_FREE_ACT);
                    if (one_in_(2))
                        add_flag(o_ptr->art_flags, TR_SEE_INVIS);
                }
                break;
            case 4: 
                one_high_resistance(o_ptr);
                break;
            case 5:
                if (abs(power) >= 2 && one_in_(2) && level >= 30)
                {
                    do { one_high_resistance(o_ptr); } while (one_in_(3));
                    break;
                }
            default:
                add_flag(o_ptr->art_flags, TR_WIS);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
            }
        }
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_PRIESTLY);
        break;
    case EGO_AMULET_TRICKERY:
        add_flag(o_ptr->art_flags, TR_SEARCH);
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(7))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_DEX);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(5, level);
                break;
            case 2:
                add_flag(o_ptr->art_flags, TR_SUST_DEX);
                break;
            case 3:
                if (one_in_(2))
                    add_flag(o_ptr->art_flags, TR_RES_POIS);
                else
                    add_flag(o_ptr->art_flags, TR_RES_DARK);
                break;
            case 4:
                if (one_in_(2))
                    add_flag(o_ptr->art_flags, TR_RES_NEXUS);
                else
                    add_flag(o_ptr->art_flags, TR_RES_CONF);
                break;
            case 5:
                if (abs(power) >= 2 && one_in_(2) && level >= 50)
                {
                    add_flag(o_ptr->art_flags, TR_TELEPATHY);
                    break;
                }
            case 6:
                if (abs(power) >= 2 && one_in_(2) && level >= 50)
                {
                    add_flag(o_ptr->art_flags, TR_SPEED);
                    o_ptr->pval = _jewelry_pval(3, level);
                    break;
                }
            default:
                add_flag(o_ptr->art_flags, TR_STEALTH);
            }
        }
        if (!o_ptr->pval) o_ptr->pval = randint1(5); /* Searching & Stealth */
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_ROGUE);
        break;
    case EGO_AMULET_HERO:
        o_ptr->to_a = randint1(5) + m_bonus(5, level);
        o_ptr->to_h = randint1(3) + m_bonus(5, level);
        o_ptr->to_d = randint1(3) + m_bonus(5, level);
        if (one_in_(3)) add_flag(o_ptr->art_flags, TR_SLOW_DIGEST);
        if (one_in_(3)) add_flag(o_ptr->art_flags, TR_SUST_CON);
        if (one_in_(3)) add_flag(o_ptr->art_flags, TR_SUST_STR);
        if (one_in_(3)) add_flag(o_ptr->art_flags, TR_SUST_DEX);
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_WARRIOR);
        break;
    case EGO_AMULET_DWARVEN:
        add_flag(o_ptr->art_flags, TR_INFRA);
        if (one_in_(2)) add_flag(o_ptr->art_flags, TR_LITE);
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(9))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_STR);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                break;
            case 2:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_DEC_DEX);
                else
                    add_flag(o_ptr->art_flags, TR_DEC_STEALTH);

                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_WIS);

                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                break;
            case 3:
                add_flag(o_ptr->art_flags, TR_CON);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                break;
            case 4:
                add_flag(o_ptr->art_flags, TR_RES_BLIND);
                break;
            case 5:
                add_flag(o_ptr->art_flags, TR_RES_DARK);
                break;
            case 6:
                add_flag(o_ptr->art_flags, TR_RES_DISEN);
                break;
            case 7:
                add_flag(o_ptr->art_flags, TR_FREE_ACT);
                break;
            default:
                add_flag(o_ptr->art_flags, TR_REGEN);
            }
        }
        if (!o_ptr->pval) o_ptr->pval = 2 + randint1(6); /* Infravision */
        break;
    case EGO_AMULET_BARBARIAN:
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(6))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_STR);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                break;
            case 2:
                add_flag(o_ptr->art_flags, TR_DEX);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                break;
            case 3:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_FREE_ACT);
                else
                {
                    add_flag(o_ptr->art_flags, TR_NO_MAGIC);
                    if (abs(power) >= 2 && one_in_(10) && level >= 70)
                    {
                        add_flag(o_ptr->art_flags, TR_MAGIC_RESISTANCE);
                        o_ptr->pval = _jewelry_pval(3, level);
                    }
                }
                break;
            case 4:
                if (abs(power) >= 2 && one_in_(10) && level >= 70)
                    add_flag(o_ptr->art_flags, TR_NO_SUMMON);
                else if (one_in_(6))
                    add_flag(o_ptr->art_flags, TR_NO_TELE);
                else
                    add_flag(o_ptr->art_flags, TR_RES_FEAR);
                break;
            case 5:
                add_flag(o_ptr->art_flags, TR_DEC_INT);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                break;
            case 6:
                o_ptr->to_a += randint1(5) + m_bonus(5, level);
                break;
            }
        }
        if (o_ptr->to_a > 15) o_ptr->to_a = 15;
        if (one_in_(ACTIVATION_CHANCE*2))
            effect_add(o_ptr, EFFECT_BERSERK);
        break;
    case EGO_AMULET_SACRED:
        add_flag(o_ptr->art_flags, TR_BLESSED);
        o_ptr->to_a = 5;
        if (one_in_(2)) add_flag(o_ptr->art_flags, TR_LITE);
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(8))
            {
            case 1:
                add_flag(o_ptr->art_flags, TR_STR);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                break;
            case 2:
                add_flag(o_ptr->art_flags, TR_WIS);
                if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                break;
            case 3:
                add_flag(o_ptr->art_flags, TR_FREE_ACT);
                if (one_in_(2))
                    add_flag(o_ptr->art_flags, TR_SEE_INVIS);
                break;
            case 4:
                if (abs(power) >= 2 && one_in_(10) && level >= 50)
                    add_flag(o_ptr->art_flags, TR_REFLECT);
                else if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_RES_CHAOS);
                else if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_RES_CONF);
                else if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_RES_NETHER);
                else
                    add_flag(o_ptr->art_flags, TR_RES_FEAR);
                break;
            case 5:
                if (abs(power) >= 2 && one_in_(20) && level >= 70)
                {
                    add_flag(o_ptr->art_flags, TR_SPEED);
                    o_ptr->pval = _jewelry_pval(3, level);
                }
                else if (one_in_(7) && level >= 50)
                {
                    add_flag(o_ptr->art_flags, TR_LIFE);
                    if (!o_ptr->pval) o_ptr->pval = _jewelry_pval(4, level);
                }
                else if (one_in_(2))
                    add_flag(o_ptr->art_flags, TR_REGEN);
                else
                    add_flag(o_ptr->art_flags, TR_HOLD_LIFE);
                break;
            default:
                o_ptr->to_a += randint1(5) + m_bonus(5, level);
            }
        }
        if (o_ptr->to_a > 20) o_ptr->to_a = 20;
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_LAW);
        break;
    case EGO_AMULET_HELL:
        o_ptr->curse_flags |= TRC_CURSED;
        o_ptr->to_a = -5;
        for (powers = _jewelry_powers(5, level, power); powers > 0; --powers)
        {
            switch (randint1(7))
            {
            case 1:
                if (one_in_(3))
                {
                    add_flag(o_ptr->art_flags, TR_AGGRAVATE);
                    one_demon_resistance(o_ptr);
                }
                else
                {
                    add_flag(o_ptr->art_flags, TR_DEC_STEALTH);
                    add_flag(o_ptr->art_flags, TR_DEC_WIS);
                    if (!o_ptr->pval) o_ptr->pval = randint1(7);
                }
                break;
            case 2:
                o_ptr->to_a -= randint1(5) + m_bonus(5, level);
                one_demon_resistance(o_ptr);
                break;
            case 3:
                add_flag(o_ptr->art_flags, TR_FREE_ACT);
                if (one_in_(2))
                    add_flag(o_ptr->art_flags, TR_SEE_INVIS);
                if (one_in_(6))
                {
                    add_flag(o_ptr->art_flags, TR_VULN_COLD);
                    o_ptr->to_h += randint1(3);
                    o_ptr->to_d += randint1(5);
                    o_ptr->to_a -= randint1(5);
                    one_demon_resistance(o_ptr);
                }
                break;
            case 4:
                o_ptr->to_h += randint1(5) + m_bonus(5, level);
                break;
            case 5:
                o_ptr->to_d += randint1(5) + m_bonus(5, level);
                if (one_in_(6))
                {
                    add_flag(o_ptr->art_flags, TR_DRAIN_EXP);
                    one_demon_resistance(o_ptr);
                }
                break;
            case 6:
                if (abs(power) >= 2 && one_in_(66) && level >= 66)
                {
                    add_flag(o_ptr->art_flags, TR_TY_CURSE);
                    add_flag(o_ptr->art_flags, TR_IM_FIRE);
                    o_ptr->to_h += randint1(6);
                    o_ptr->to_d += randint1(6);
                    o_ptr->to_a -= randint1(20);
                    break;
                }
                else if (one_in_(3))
                {
                    add_flag(o_ptr->art_flags, TR_DEC_SPEED);
                    o_ptr->pval = randint1(3);
                    o_ptr->to_h += randint1(3);
                    o_ptr->to_d += randint1(5);
                    o_ptr->to_a -= randint1(5);
                    one_demon_resistance(o_ptr);
                    break;
                }
            default:
                o_ptr->to_h += randint1(3);
                o_ptr->to_d += randint1(5);
            }
        }
        if (o_ptr->to_a < -20) o_ptr->to_a = -20;
        if (o_ptr->to_h > 20) o_ptr->to_h = 20;
        if (o_ptr->to_d > 16) o_ptr->to_d = 16;
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_DEMON);
        break;
    case EGO_AMULET_ELEMENTAL:
        if (abs(power) >= 2 && randint1(level) > 30)
        {
            add_flag(o_ptr->art_flags, TR_RES_COLD);
            add_flag(o_ptr->art_flags, TR_RES_FIRE);
            add_flag(o_ptr->art_flags, TR_RES_ELEC);
            if (one_in_(3))
                add_flag(o_ptr->art_flags, TR_RES_ACID);
            if (one_in_(5))
                add_flag(o_ptr->art_flags, TR_RES_POIS);
            else if (one_in_(5))
                add_flag(o_ptr->art_flags, TR_RES_SHARDS);
        }
        else
        {
            one_ele_resistance(o_ptr);
            if (one_in_(3))
                one_ele_resistance(o_ptr);
        }
        if (one_in_(ACTIVATION_CHANCE))
            effect_add_random(o_ptr, BIAS_ELEMENTAL);
        break;
    case EGO_AMULET_DEFENDER:
        _create_defender(o_ptr, level, power);
        break;
    }

    _finalize_jewelry(o_ptr);

    /* Be sure to cursify later! */
    if (power == -1)
        power--;
}

static bool _create_device(object_type *o_ptr, int level, int power, int mode)
{
    /* Create the device and pick the effect. This can fail if, for example,
       mode is AM_GOOD and we are too shallow for any of the good effects. */
    if (!device_init(o_ptr, level, mode))
        return FALSE;

    if (abs(power) > 1)
    {
        bool done = FALSE;
        u32b flgs[TR_FLAG_SIZE];

        object_flags(o_ptr, flgs);

        while (!done)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_DEVICE);
            done = TRUE;

            if ( o_ptr->name2 == EGO_DEVICE_RESISTANCE
              && (have_flag(flgs, TR_IGNORE_ACID) || o_ptr->tval == TV_ROD) )
            {
                done = FALSE;
            }
        }

        switch (o_ptr->name2)
        {
        case EGO_DEVICE_CAPACITY:
            o_ptr->pval = 1 + m_bonus(4, level);
            o_ptr->xtra4 += o_ptr->xtra4 * o_ptr->pval * 10 / 100;
            break;
        case EGO_DEVICE_SIMPLICITY:
        case EGO_DEVICE_POWER:
        case EGO_DEVICE_REGENERATION:
        case EGO_DEVICE_QUICKNESS:
            o_ptr->pval = 1 + m_bonus(4, level);
            break;
        }
    }

    if (power < 0)
        o_ptr->curse_flags |= TRC_CURSED;

    return TRUE;
}
void adjust_weapon_weight(object_type *o_ptr)
{
    /* Experimental: Maulers need heavy weapons!
        Anything that dice boosts gets heavier. */
    if (object_is_melee_weapon(o_ptr) && p_ptr->pclass == CLASS_MAULER)
    {
    object_kind *k_ptr = &k_info[o_ptr->k_idx];
    int          dice = o_ptr->dd * o_ptr->ds;
    int          orig = k_ptr->dd * k_ptr->ds;
        
        if (dice > orig)
        {
            int wgt = o_ptr->weight;
            int xtra = k_ptr->weight;
            int mult = (dice - orig) * 100 / orig;

            while (mult >= 100)
            {
                xtra = xtra * 3 / 4;
                wgt += xtra;
                mult -= 100;
            }
            if (mult > 0)
            {
                xtra = xtra * 3 / 4;
                wgt += xtra * mult / 100;
            }

            o_ptr->weight = wgt;

            /*  a Bo Staff (1d11) ... 16.0 lbs
                a Bo Staff (2d12) ... 29.6 lbs
                a Bo Staff (3d12) ... 38.8 lbs
                a Bo Staff (4d12) ... 45.5 lbs
                a Bo Staff (5d12) ... 50.3 lbs
                a Bo Staff (6d12) ... 53.8 lbs
                a Bo Staff (7d12) ... 56.3 lbs

                a Heavy Lance (4d8) ... 40.0 lbs
                a Heavy Lance (5d8) ... 47.5 lbs
                a Heavy Lance (6d8) ... 55.0 lbs
                a Heavy Lance (8d8) ... 70.0 lbs
                a Heavy Lance (8d9) ... 75.6 lbs

                a Dagger (1d4) ... 1.2 lbs
                a Dagger (2d4) ... 2.1 lbs
                a Dagger (3d4) ... 2.7 lbs
                a Dagger (7d5) ... 3.7 lbs
            */
        }
    }
}

static void _create_weapon(object_type *o_ptr, int level, int power, int mode)
{
    int tohit1 = randint1(5) + m_bonus(5, level);
    int todam1 = randint1(5) + m_bonus(5, level);

    int tohit2 = m_bonus(10, level);
    int todam2 = m_bonus(10, level);
    bool crafting = (mode & AM_CRAFTING) ? TRUE : FALSE;
    bool done = FALSE;

    if ((o_ptr->tval == TV_BOLT) || (o_ptr->tval == TV_ARROW) || (o_ptr->tval == TV_SHOT))
    {
        tohit2 = (tohit2+1)/2;
        todam2 = (todam2+1)/2;
    }

    if (!crafting)
    {
        if (power >= 2 && o_ptr->tval == TV_SWORD && o_ptr->sval == SV_DIAMOND_EDGE && !one_in_(7)) return;
    }
    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_POISON_NEEDLE) return;

    if (!crafting)
    {
        if (power == -1)
        {
            o_ptr->to_h -= tohit1;
            o_ptr->to_d -= todam1;
            if (power < -1)
            {
                o_ptr->to_h -= tohit2;
                o_ptr->to_d -= todam2;
            }

            if (o_ptr->to_h + o_ptr->to_d < 0) o_ptr->curse_flags |= TRC_CURSED;
        }
        else if (power)
        {
            o_ptr->to_h += tohit1;
            o_ptr->to_d += todam1;
            if (power > 1 || power < -1)
            {
                o_ptr->to_h += tohit2;
                o_ptr->to_d += todam2;
            }
        }
    }

    if (-1 <= power && power <= 1)
        return;

    if (mode & AM_FORCE_EGO)
        crafting = TRUE; /* Hack to prevent artifacts */

    switch (o_ptr->tval)
    {
    case TV_BOW:
        if (o_ptr->sval == SV_HARP)
        {
            o_ptr->to_h = 0;
            o_ptr->to_d = 0;
        }

        if ((!crafting && one_in_(20)) || power > 2)
        {
            create_artifact(o_ptr, CREATE_ART_NORMAL);
            break;
        }
        if (o_ptr->sval == SV_HARP)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_HARP);
            break;
        }
        
        while (!done)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_BOW);
            done = TRUE;

            switch (o_ptr->name2)
            {
            case EGO_BOW_VELOCITY:
                o_ptr->mult  += 25;
                break;
            case EGO_BOW_EXTRA_MIGHT:
                o_ptr->mult  += 25 + m_bonus(15, level) * 5;
                break;
            case EGO_BOW_LOTHLORIEN:
                if (o_ptr->sval != SV_LONG_BOW)
                    done = FALSE;
                else
                {
                    o_ptr->mult  += 25 + m_bonus(17, level) * 5;

                    if (one_in_(3))
                        add_flag(o_ptr->art_flags, TR_XTRA_SHOTS);
                    else
                        one_high_resistance(o_ptr);
                }
                break;
            case EGO_BOW_BUCKLAND:
                if (o_ptr->sval != SV_SLING)
                    done = FALSE;
                else
                {
                    if (one_in_(3))
                        o_ptr->mult  += 25 + m_bonus(15, level) * 5;
                    else
                        one_high_resistance(o_ptr);
                }
                break;
            case EGO_BOW_HARADRIM:
                if (o_ptr->sval != SV_HEAVY_XBOW)
                    done = FALSE;
                else
                {
                    o_ptr->mult  += 25 + m_bonus(20, level) * 5;
                    if (one_in_(3))
                    {
                        add_flag(o_ptr->art_flags, TR_XTRA_SHOTS);
                        add_flag(o_ptr->art_flags, TR_DEC_SPEED);
                    }
                    else
                        one_high_resistance(o_ptr);
                }
                break;
            }
        }
        break;

    case TV_BOLT:
    case TV_ARROW:
    case TV_SHOT:
        if (power < 0)
            break;

        o_ptr->name2 = _get_random_ego(EGO_TYPE_AMMO);

        switch (o_ptr->name2)
        {
        case EGO_AMMO_SLAYING:
            o_ptr->dd++;
            break;
        }

        /* Hack -- super-charge the damage dice */
        while (one_in_(10 * o_ptr->dd * o_ptr->ds)) 
            o_ptr->dd++;

        if (o_ptr->dd > 9) 
            o_ptr->dd = 9;
        break;

    case TV_DIGGING:
        if ((!crafting && one_in_(30)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }
        while (!done)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_DIGGER);
            done = TRUE;
            switch (o_ptr->name2)
            {
            case EGO_DIGGER_DISSOLVING:
                o_ptr->dd += 1;
                break;
            case EGO_DIGGER_DISRUPTION:
                if (o_ptr->sval != SV_MATTOCK)
                    done = FALSE;
                else
                    o_ptr->dd += 2;
                break;
            }
        }
        break;
        
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_SWORD:
        if ((!crafting && one_in_(40)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }
        while (!done)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_WEAPON);
            done = TRUE;
            switch (o_ptr->name2)
            {
            case EGO_WEAPON_BURNING:
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_FIRE);
                break;
            case EGO_WEAPON_FREEZING:
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_COLD);
                break;
            case EGO_WEAPON_MELTING:
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_ACID);
                break;
            case EGO_WEAPON_SHOCKING:
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_ELEC);
                break;
            case EGO_WEAPON_VENOM:
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_POIS);
                break;
            case EGO_WEAPON_CHAOS:
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_CHAOS);
                break;
            case EGO_WEAPON_ARCANE:
                if (o_ptr->tval != TV_HAFTED || o_ptr->sval != SV_WIZSTAFF)
                    done = FALSE;
                else
                {
                    o_ptr->pval = randint1(2);
                    if (one_in_(30))
                        o_ptr->pval++;
                    o_ptr->to_h = -10;
                    o_ptr->to_d = -10;
                    if (one_in_(ACTIVATION_CHANCE))
                        effect_add_random(o_ptr, BIAS_MAGE);
                }
                break;
            case EGO_WEAPON_ARMAGEDDON:
            {
                int odds = o_ptr->dd * o_ptr->ds / 2;
                    
                if (odds < 3) odds = 3;
                if (one_in_(odds)) /* double damage */
                {
                    o_ptr->dd *= 2;

                    /* Look alikes to keep players happy */
                    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_LONG_SWORD && one_in_(2))
                        o_ptr->dd = 5; /* Vorpal Blade */
                    if (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_KATANA)
                    {
                        o_ptr->dd = 8; /* Aglarang */
                        o_ptr->ds = 4;
                        if (one_in_(100))
                        {
                            o_ptr->dd = 10;
                            o_ptr->ds = 5; /* Muramasa */
                        }
                    }
                    if (o_ptr->tval == TV_HAFTED && o_ptr->sval == SV_WAR_HAMMER)
                        o_ptr->dd = 9; /* Aule */
                }
                else
                {
                    do
                    {
                        o_ptr->dd++;
                    }
                    while (one_in_(o_ptr->dd));
                        
                    do
                    {
                        o_ptr->ds++;
                    }
                    while (one_in_(o_ptr->ds));
                }
                    
                if (one_in_(5))
                {
                    switch (randint1(5))
                    {
                    case 1: add_flag(o_ptr->art_flags, TR_BRAND_ELEC); break;
                    case 2: add_flag(o_ptr->art_flags, TR_BRAND_FIRE); break;
                    case 3: add_flag(o_ptr->art_flags, TR_BRAND_COLD); break;
                    case 4: add_flag(o_ptr->art_flags, TR_BRAND_ACID); break;
                    default: add_flag(o_ptr->art_flags, TR_BRAND_POIS); break;
                    }
                }

                if (o_ptr->tval == TV_SWORD && one_in_(3))
                    add_flag(o_ptr->art_flags, TR_VORPAL);

                if (o_ptr->tval == TV_HAFTED && one_in_(7))
                    add_flag(o_ptr->art_flags, TR_IMPACT);

                if (one_in_(666))
                    add_flag(o_ptr->art_flags, TR_FORCE_WEAPON);

                break;
            }
            case EGO_WEAPON_CRUSADE:
                if (one_in_(4) && (level > 40))
                    add_flag(o_ptr->art_flags, TR_BLOWS);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_PRIESTLY);
                break;
            case EGO_WEAPON_DAEMON:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_SLAY_GOOD);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_AGGRAVATE);
                else
                    add_flag(o_ptr->art_flags, TR_DEC_STEALTH);
                if (p_ptr->pclass == CLASS_PRIEST && (p_ptr->realm1 == REALM_DAEMON || p_ptr->realm2 == REALM_DAEMON))
                    add_flag(o_ptr->art_flags, TR_BLESSED);
                break;
            case EGO_WEAPON_DEATH:
                if (one_in_(6))
                    add_flag(o_ptr->art_flags, TR_VULN_LITE);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_SLAY_GOOD);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_RES_NETHER);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_SLAY_HUMAN);
                else if (one_in_(13))
                {
                    add_flag(o_ptr->art_flags, TR_SLAY_LIVING);
                    o_ptr->dd++; 
                    o_ptr->curse_flags |= TRC_CURSED;
                    o_ptr->curse_flags |= get_curse(2, o_ptr);
                }
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_NECROMANTIC);
                if (p_ptr->pclass == CLASS_PRIEST && (p_ptr->realm1 == REALM_DEATH || p_ptr->realm2 == REALM_DEATH))
                    add_flag(o_ptr->art_flags, TR_BLESSED);
                break;
            case EGO_WEAPON_MORGUL:
                if (p_ptr->pclass == CLASS_PRIEST && (p_ptr->realm1 == REALM_DEATH || p_ptr->realm2 == REALM_DEATH))
                    add_flag(o_ptr->art_flags, TR_BLESSED);
                break;
            case EGO_WEAPON_DEFENDER:
                o_ptr->to_a = 5;
                if (one_in_(4))
                {
                    int i;
                    int ct = 4;
                    if (one_in_(3)) 
                        ct++;

                    for (i = 0; i < ct; i++)
                        one_high_resistance(o_ptr);
                }
                else
                {
                    add_flag(o_ptr->art_flags, TR_RES_FIRE);
                    add_flag(o_ptr->art_flags, TR_RES_COLD);
                    add_flag(o_ptr->art_flags, TR_RES_ACID);
                    add_flag(o_ptr->art_flags, TR_RES_ELEC);
                    if (one_in_(3)) 
                        add_flag(o_ptr->art_flags, TR_RES_POIS);
                }
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_WARNING);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_LEVITATION);
                if (one_in_(7))
                    add_flag(o_ptr->art_flags, TR_REGEN);
                break;
            case EGO_WEAPON_EARTHQUAKES:
                if (o_ptr->tval != TV_HAFTED)
                    done = FALSE;
                else
                    o_ptr->pval = m_bonus(3, level);
                break;
            case EGO_WEAPON_KILL_DEMON:
                if (one_in_(3))
                    one_demon_resistance(o_ptr);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_KILL_DEMON);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_DEMON);
                break;
            case EGO_WEAPON_KILL_DRAGON:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_RES_POIS);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_KILL_DRAGON);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_ELEMENTAL);
                break;
            case EGO_WEAPON_KILL_EVIL:
                if (one_in_(30))
                    add_flag(o_ptr->art_flags, TR_KILL_EVIL);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_LAW);
                break;
            case EGO_WEAPON_KILL_GIANT:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_SUST_STR);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_KILL_GIANT);
                if (one_in_(ACTIVATION_CHANCE*2)) /* TODO: Need more "Giant" activations */
                    effect_add_random(o_ptr, BIAS_STR);
                break;
            case EGO_WEAPON_KILL_HUMAN:
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_KILL_HUMAN);
                break;
            case EGO_WEAPON_KILL_ORC:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_KILL_ORC);
                break;
            case EGO_WEAPON_KILL_TROLL:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_REGEN);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_KILL_TROLL);
                break;
            case EGO_WEAPON_KILL_UNDEAD:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_HOLD_LIFE);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_KILL_UNDEAD);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_NECROMANTIC);
                break;
            case EGO_WEAPON_NATURE:
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_KILL_ANIMAL);
                if (one_in_(ACTIVATION_CHANCE))
                    effect_add_random(o_ptr, BIAS_RANGER);
                break;
            case EGO_WEAPON_NOLDOR:
                if ( o_ptr->tval != TV_SWORD 
                  || o_ptr->sval == SV_BLADE_OF_CHAOS
                  || o_ptr->dd * o_ptr->ds < 10 )
                {
                    done = FALSE;
                }
                else
                {
                    o_ptr->dd += 1;
                }
                break;
            case EGO_WEAPON_ORDER:
                o_ptr->dd = o_ptr->dd * o_ptr->ds;
                o_ptr->ds = 1;
                break;
            case EGO_WEAPON_PATTERN:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_HOLD_LIFE);
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_DEX);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_RES_FEAR);
                break;
            case EGO_WEAPON_SHARPNESS:
                if (o_ptr->tval != TV_SWORD)
                    done = FALSE;
                else
                {
                    o_ptr->pval = m_bonus(5, level) + 1;
                    if (one_in_(2))
                    {
                        do
                        {
                            o_ptr->dd++;
                        }
                        while (one_in_(o_ptr->dd));
                    }
                    if (one_in_(7))
                        add_flag(o_ptr->art_flags, TR_VORPAL2);
                    else
                        add_flag(o_ptr->art_flags, TR_VORPAL);
                }
                break;
            case EGO_WEAPON_TRUMP:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_CHR);
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_SLAY_DEMON);
                if (one_in_(7))
                    one_ability(o_ptr);
                break;
            case EGO_WEAPON_WESTERNESSE:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_RES_FEAR);
                break;
            case EGO_WEAPON_WILD:
                o_ptr->ds = o_ptr->dd * o_ptr->ds;
                o_ptr->dd = 1;
                break;
            case EGO_WEAPON_JOUSTING:
                if ( !object_is_(o_ptr, TV_POLEARM, SV_LANCE)
                  && !object_is_(o_ptr, TV_POLEARM, SV_HEAVY_LANCE) )
                {
                    done = FALSE;
                }
                else
                {
                    while (one_in_(o_ptr->dd * 3)) { o_ptr->dd++; }
                    if (one_in_(3))
                        add_flag(o_ptr->art_flags, TR_SLAY_HUMAN);
                }
                break;
            case EGO_WEAPON_HELL_LANCE:
                if ( !object_is_(o_ptr, TV_POLEARM, SV_LANCE)
                  && !object_is_(o_ptr, TV_POLEARM, SV_HEAVY_LANCE) )
                {
                    done = FALSE;
                }
                else
                {
                    while (one_in_(o_ptr->dd * 4)) { o_ptr->dd++; }
                    one_demon_resistance(o_ptr);
                    if (one_in_(16))
                        add_flag(o_ptr->art_flags, TR_VAMPIRIC);
                    if (one_in_(ACTIVATION_CHANCE))
                        effect_add_random(o_ptr, BIAS_DEMON);
                }
                break;
            case EGO_WEAPON_HOLY_LANCE:
                if ( !object_is_(o_ptr, TV_POLEARM, SV_LANCE)
                  && !object_is_(o_ptr, TV_POLEARM, SV_HEAVY_LANCE) )
                {
                    done = FALSE;
                }
                else
                {
                    while (one_in_(o_ptr->dd * 5)) { o_ptr->dd++; }
                    one_holy_resistance(o_ptr);
                    if (one_in_(77))
                    {
                        o_ptr->dd = o_ptr->dd * o_ptr->ds;
                        o_ptr->ds = 1;
                        add_flag(o_ptr->art_flags, TR_ORDER);

                    }
                    if (one_in_(ACTIVATION_CHANCE))
                        effect_add_random(o_ptr, BIAS_PRIESTLY);
                }
                break;
            }
        }
        /* Hack -- Super-charge the damage dice, but only if they haven't already
           been boosted/altered by the ego type (e.g., Armageddon, Hell Lance, Wild, Order, etc) */
        if ( !o_ptr->art_name 
          && o_ptr->name2 /* These first two checks are paranoia, pure and simple! */
          && o_ptr->dd == k_info[o_ptr->k_idx].dd
          && o_ptr->ds == k_info[o_ptr->k_idx].ds
          && o_ptr->name2 != EGO_WEAPON_EXTRA_ATTACKS
          && o_ptr->name2 != EGO_WEAPON_WILD
          && o_ptr->name2 != EGO_WEAPON_ORDER )
        {
            if (o_ptr->dd * o_ptr->ds > 0 && one_in_(5 + 200/MAX(level, 1)))
            {
                do
                {
                    o_ptr->dd++;
                }
                while (one_in_(o_ptr->dd * o_ptr->ds / 2));
            }
        }

        if (done)
            adjust_weapon_weight(o_ptr);
        break;
    }
}


bool add_esp_strong(object_type *o_ptr)
{
    bool nonliv = FALSE;

    switch (randint1(3))
    {
    case 1: add_flag(o_ptr->art_flags, TR_ESP_EVIL); break;
    case 2: add_flag(o_ptr->art_flags, TR_TELEPATHY); break;
    case 3:    add_flag(o_ptr->art_flags, TR_ESP_NONLIVING); nonliv = TRUE; break;
    }

    return nonliv;
}


#define MAX_ESP_WEAK 9
void add_esp_weak(object_type *o_ptr, bool extra)
{
    int i = 0;
    int idx[MAX_ESP_WEAK];
    int flg[MAX_ESP_WEAK];
    int n = (extra) ? (3 + randint1(randint1(6))) : randint1(3);
    int left = MAX_ESP_WEAK;

    for (i = 0; i < MAX_ESP_WEAK; i++) flg[i] = i + 1;

    /* Shuffle esp flags */
    for (i = 0; i < n; i++)
    {
        int k = randint0(left--);

        idx[i] = flg[k];

        while (k < left)
        {
            flg[k] = flg[k + 1];
            k++;
        }
    }

    while (n--) switch (idx[n])
    {
    case 1: add_flag(o_ptr->art_flags, TR_ESP_ANIMAL); break;
    case 2: add_flag(o_ptr->art_flags, TR_ESP_UNDEAD); break;
    case 3: add_flag(o_ptr->art_flags, TR_ESP_DEMON); break;
    case 4: add_flag(o_ptr->art_flags, TR_ESP_ORC); break;
    case 5: add_flag(o_ptr->art_flags, TR_ESP_TROLL); break;
    case 6: add_flag(o_ptr->art_flags, TR_ESP_GIANT); break;
    case 7: add_flag(o_ptr->art_flags, TR_ESP_DRAGON); break;
    case 8: add_flag(o_ptr->art_flags, TR_ESP_HUMAN); break;
    case 9: add_flag(o_ptr->art_flags, TR_ESP_GOOD); break;
    }
}

static int *_effect_list = NULL;
static bool _effect_list_p(int effect)
{
    int i;
    assert(_effect_list);
    for (i = 0; ; i++)
    {
        int n = _effect_list[i];
        if (n == -1) return FALSE;
        if (n == effect) return TRUE;
     }
    /* return FALSE;  unreachable */
}

static void _effect_add_list(object_type *o_ptr, int *list)
{
    _effect_list = list;
    effect_add_random_p(o_ptr, _effect_list_p);
}

static void _create_armor(object_type *o_ptr, int level, int power, int mode)
{
    int toac1 = randint1(5) + m_bonus(5, level);
    int toac2 = m_bonus(10, level);
    bool crafting = (mode & AM_CRAFTING) ? TRUE : FALSE;
    bool done = FALSE;

    if (!crafting)
    {
        if (power == -1)
        {
            o_ptr->to_a -= toac1;
            if (power < -1)
                o_ptr->to_a -= toac2;

            if (o_ptr->to_a < 0) o_ptr->curse_flags |= TRC_CURSED;
        }
        else if (power)
        {
            o_ptr->to_a += toac1;
            if (power > 1 || power < -1)
                o_ptr->to_a += toac2;
        }
    }

    if (-1 <= power && power <= 1)
        return;

    if (mode & AM_FORCE_EGO)
        crafting = TRUE; /* Hack to prevent artifacts */

    switch (o_ptr->tval)
    {
    case TV_DRAG_ARMOR:
        if ((!crafting && one_in_(50)) || power > 2)
            _create_artifact(o_ptr, power);
        if (cheat_peek) object_mention(o_ptr);
        break;

    case TV_GLOVES:
        if ((!crafting && one_in_(20)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }
        o_ptr->name2 = _get_random_ego(EGO_TYPE_GLOVES);
        switch (o_ptr->name2)
        {
        case EGO_GLOVES_GIANT:
            if (one_in_(4))
            {
                switch (randint1(3))
                {
                case 1: add_flag(o_ptr->art_flags, TR_RES_SOUND); break;
                case 2: add_flag(o_ptr->art_flags, TR_RES_SHARDS); break;
                case 3: add_flag(o_ptr->art_flags, TR_RES_CHAOS); break;
                }
            }
            if (one_in_(3))
                add_flag(o_ptr->art_flags, TR_VULN_CONF);
            if (one_in_(2))
                add_flag(o_ptr->art_flags, TR_DEC_STEALTH);
            if (one_in_(2))
                add_flag(o_ptr->art_flags, TR_DEC_DEX);
            break;
        case EGO_GLOVES_WIZARD:
            if (one_in_(4))
            {
                switch (randint1(3))
                {
                case 1: add_flag(o_ptr->art_flags, TR_RES_CONF); break;
                case 2: add_flag(o_ptr->art_flags, TR_RES_BLIND); break;
                case 3: add_flag(o_ptr->art_flags, TR_RES_LITE); break;
                }
            }
            if (one_in_(2))
                add_flag(o_ptr->art_flags, TR_DEC_STR);
            if (one_in_(3))
                add_flag(o_ptr->art_flags, TR_DEC_CON);
            if (one_in_(30))
                add_flag(o_ptr->art_flags, TR_DEVICE_POWER);
            break;
        case EGO_GLOVES_YEEK:
            if (one_in_(10))
                add_flag(o_ptr->art_flags, TR_IM_ACID);
            break;
        case EGO_GLOVES_THIEF:
            if (one_in_(20))
                add_flag(o_ptr->art_flags, TR_SPEED);
            break;
        case EGO_GLOVES_BERSERKER:
            o_ptr->to_h = -10;
            o_ptr->to_d = 10;
            o_ptr->to_a = -10;
            break;
        }
        break;

    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
        if (object_is_(o_ptr, TV_SOFT_ARMOR, SV_ROBE) && one_in_(7))
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_ROBE);
            switch (o_ptr->name2)
            {
            case EGO_ROBE_TWILIGHT:
                o_ptr->k_idx = lookup_kind(TV_SOFT_ARMOR, SV_YOIYAMI_ROBE);
                o_ptr->sval = SV_YOIYAMI_ROBE;
                o_ptr->ac = 0;
                o_ptr->to_a = 0;
                break;
            case EGO_ROBE_SORCERER:
                one_high_resistance(o_ptr);
                one_high_resistance(o_ptr);
                one_high_resistance(o_ptr);
                break;
            }
            break;
        }

        if ((!crafting && one_in_(20)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }

        while (!done)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_BODY_ARMOR);
            done = TRUE;

            switch (o_ptr->name2)
            {
            case EGO_BODY_PROTECTION:
                if (one_in_(3))
                    o_ptr->to_a += m_bonus(10, level);
                break;
            case EGO_BODY_ELEMENTAL_PROTECTION:
                one_ele_resistance(o_ptr);
                do { one_ele_resistance(o_ptr); } while (one_in_(2));
                if (one_in_(4))
                    add_flag(o_ptr->art_flags, TR_RES_POIS);
                break;
            case EGO_BODY_CELESTIAL_PROTECTION:
                one_high_resistance(o_ptr);
                do { one_high_resistance(o_ptr); } while (one_in_(2));
                if (one_in_(7))
                    add_flag(o_ptr->art_flags, TR_HOLD_LIFE);
                if (one_in_(17))
                    add_flag(o_ptr->art_flags, TR_REFLECT);
                break;
            case EGO_BODY_ELVENKIND:
                if (one_in_(4))
                {
                    add_flag(o_ptr->art_flags, TR_DEC_STR);
                    add_flag(o_ptr->art_flags, TR_DEX);
                }
                break;
            case EGO_BODY_DWARVEN:
                if (o_ptr->tval != TV_HARD_ARMOR || o_ptr->sval == SV_RUSTY_CHAIN_MAIL)
                    done = FALSE;
                else
                {
                    o_ptr->weight = (2 * k_info[o_ptr->k_idx].weight / 3);
                    o_ptr->ac = k_info[o_ptr->k_idx].ac + 5;
                    if (one_in_(4))
                        add_flag(o_ptr->art_flags, TR_CON);
                    if (one_in_(4))
                        add_flag(o_ptr->art_flags, TR_DEC_DEX);
                    if (one_in_(4))
                        add_flag(o_ptr->art_flags, TR_DEC_STEALTH);
                    if (one_in_(2))
                        add_flag(o_ptr->art_flags, TR_REGEN);
                }
                break;
            case EGO_BODY_URUK_HAI:
                if (o_ptr->tval != TV_HARD_ARMOR)
                    done = FALSE;
                else
                {
                    if (one_in_(4))
                        add_flag(o_ptr->art_flags, TR_DEC_STEALTH);
                }
                break;
            case EGO_BODY_OLOG_HAI:
                if (o_ptr->tval != TV_HARD_ARMOR)
                    done = FALSE;
                else
                {
                    if (one_in_(4))
                        add_flag(o_ptr->art_flags, TR_CON);
                    if (one_in_(4))
                        add_flag(o_ptr->art_flags, TR_DEC_STEALTH);
                }
                break;
            case EGO_BODY_DEMON:
            case EGO_BODY_DEMON_LORD:
                if (o_ptr->tval != TV_HARD_ARMOR)
                    done = FALSE;
                else
                {
                    if (one_in_(ACTIVATION_CHANCE))
                        effect_add_random(o_ptr, BIAS_DEMON);
                }
                break;
            }
        }
        break;

    case TV_SHIELD:
        if ((!crafting && one_in_(20)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }

        while (!done)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_SHIELD);
            done = TRUE;

            switch (o_ptr->name2)
            {
            case EGO_SHIELD_PROTECTION:
                /*o_ptr->to_a += 7;*/
                break;
            case EGO_SHIELD_ELEMENTAL_PROTECTION:
                one_ele_resistance(o_ptr);
                do { one_ele_resistance(o_ptr); } while (one_in_(3));
                if (one_in_(5))
                    add_flag(o_ptr->art_flags, TR_RES_POIS);
                break;
            case EGO_SHIELD_CELESTIAL_PROTECTION:
                one_high_resistance(o_ptr);
                do { one_high_resistance(o_ptr); } while (one_in_(3));
                if (one_in_(17))
                    add_flag(o_ptr->art_flags, TR_REFLECT);
                break;
            case EGO_SHIELD_ELVENKIND:
                if (one_in_(4))
                    add_flag(o_ptr->art_flags, TR_DEC_STR);
                if (one_in_(4))
                    add_flag(o_ptr->art_flags, TR_DEX);
                break;
            case EGO_SHIELD_REFLECTION:
                if (o_ptr->sval == SV_MIRROR_SHIELD)
                    done = FALSE;
                break;
            case EGO_SHIELD_ORCISH:
                if ( o_ptr->sval == SV_DRAGON_SHIELD 
                  || o_ptr->sval == SV_MIRROR_SHIELD )
                {
                    done = FALSE;
                }
                break;
            case EGO_SHIELD_DWARVEN:
                if ( o_ptr->sval == SV_SMALL_LEATHER_SHIELD
                  || o_ptr->sval == SV_LARGE_LEATHER_SHIELD
                  || o_ptr->sval == SV_DRAGON_SHIELD 
                  || o_ptr->sval == SV_MIRROR_SHIELD )
                {
                    done = FALSE;
                }
                else
                {
                    o_ptr->weight = (2 * k_info[o_ptr->k_idx].weight / 3);
                    o_ptr->ac = k_info[o_ptr->k_idx].ac + 4;
                    if (one_in_(4))
                        add_flag(o_ptr->art_flags, TR_SUST_CON);
                }
                break;
            }
        }
        break;

    case TV_CROWN:
        if ((!crafting && one_in_(20)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }
        o_ptr->name2 = _get_random_ego(EGO_TYPE_CROWN);
        switch (o_ptr->name2)
        {
        case EGO_CROWN_TELEPATHY:
            if (add_esp_strong(o_ptr)) add_esp_weak(o_ptr, TRUE);
            else add_esp_weak(o_ptr, FALSE);
            break;
        case EGO_CROWN_MAGI:
            if (one_in_(7))
                add_flag(o_ptr->art_flags, TR_EASY_SPELL);
            if (one_in_(5))
                add_flag(o_ptr->art_flags, TR_MAGIC_MASTERY);
            else if (one_in_(66))
            {
                add_flag(o_ptr->art_flags, TR_SPELL_POWER);
                add_flag(o_ptr->art_flags, TR_DEC_CON);
            }
            else if (one_in_(3))
            {
                o_ptr->to_d += 4 + randint1(11);
                while (one_in_(2))
                    o_ptr->to_d++;

                add_flag(o_ptr->art_flags, TR_SHOW_MODS);
            }
            break;
        case EGO_CROWN_LORDLINESS:
            if (one_in_(7))
                add_flag(o_ptr->art_flags, TR_SPELL_CAP);
            break;
        case EGO_CROWN_MIGHT:
            if (one_in_(7))
            {
                o_ptr->to_h += randint1(7);
                o_ptr->to_d += randint1(7);
            }
            break;
        case EGO_CROWN_SEEING:
            if (one_in_(3))
            {
                if (one_in_(2)) add_esp_strong(o_ptr);
                else add_esp_weak(o_ptr, FALSE);
            }
            break;
        case EGO_CROWN_CELESTIAL_PROTECTION:
            one_high_resistance(o_ptr);
            do { one_high_resistance(o_ptr); } while (one_in_(3));
            break;
        }
        break;

    case TV_HELM:
        if ((!crafting && one_in_(20)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }
        while (!done)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_HELMET);
            done = TRUE;

            switch (o_ptr->name2)
            {
            case EGO_HELMET_SEEING:
                if (one_in_(7))
                {
                    if (one_in_(2)) add_esp_strong(o_ptr);
                    else add_esp_weak(o_ptr, FALSE);
                }
                break;
            case EGO_HELMET_DWARVEN:
                if (o_ptr->sval == SV_HARD_LEATHER_CAP || o_ptr->sval == SV_DRAGON_HELM)
                {
                    done = FALSE;
                    break;
                }
                o_ptr->weight = (2 * k_info[o_ptr->k_idx].weight / 3);
                o_ptr->ac = k_info[o_ptr->k_idx].ac + 3;
                if (one_in_(4))
                    add_flag(o_ptr->art_flags, TR_TUNNEL);
                break;

            case EGO_HELMET_SUNLIGHT:
                if (one_in_(3))
                    add_flag(o_ptr->art_flags, TR_VULN_DARK);
                if (one_in_(ACTIVATION_CHANCE))
                {
                    int choices[] = {
                        EFFECT_LITE_AREA, EFFECT_LITE_MAP_AREA, EFFECT_BOLT_LITE, EFFECT_BEAM_LITE_WEAK,
                        EFFECT_BEAM_LITE, EFFECT_BALL_LITE, EFFECT_BREATHE_LITE, EFFECT_CONFUSING_LITE, -1 
                    };
                    _effect_add_list(o_ptr, choices);
                }
                break;

            case EGO_HELMET_KNOWLEDGE:
                if (one_in_(7))
                    add_flag(o_ptr->art_flags, TR_MAGIC_MASTERY);
                if (one_in_(ACTIVATION_CHANCE))
                {
                    int choices[] = {
                        EFFECT_IDENTIFY, EFFECT_IDENTIFY_FULL, EFFECT_PROBING, EFFECT_DETECT_TRAPS, 
                        EFFECT_DETECT_MONSTERS, EFFECT_DETECT_OBJECTS, EFFECT_DETECT_ALL, 
                        EFFECT_ENLIGHTENMENT, EFFECT_CLAIRVOYANCE, EFFECT_SELF_KNOWLEDGE, -1 
                    };
                    _effect_add_list(o_ptr, choices);
                }
                break;
            case EGO_HELMET_PIETY:
                if (one_in_(ACTIVATION_CHANCE))
                {
                    int choices[] = {
                        EFFECT_HEAL, EFFECT_CURING, EFFECT_RESTORE_STATS, EFFECT_RESTORE_EXP, 
                        EFFECT_HEAL_CURING, EFFECT_CURE_POIS, EFFECT_CURE_FEAR, 
                        EFFECT_REMOVE_CURSE, EFFECT_REMOVE_ALL_CURSE, EFFECT_CLARITY, -1 
                    };
                    _effect_add_list(o_ptr, choices);
                }
                break;
            case EGO_HELMET_RAGE:
                o_ptr->to_d += 3;
                o_ptr->to_d += m_bonus(7, level);
                break;
            }
        }
        break;

    case TV_CLOAK:
        if ((!crafting && one_in_(20)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }
        o_ptr->name2 = _get_random_ego(EGO_TYPE_CLOAK);
        switch (o_ptr->name2)
        {
        case EGO_CLOAK_IMMOLATION:
            if (one_in_(ACTIVATION_CHANCE))
                effect_add_random(o_ptr, BIAS_FIRE);
            break;
        case EGO_CLOAK_ELECTRICITY:
            if (one_in_(ACTIVATION_CHANCE))
                effect_add_random(o_ptr, BIAS_ELEC);
            break;
        case EGO_CLOAK_FREEZING:
            if (one_in_(ACTIVATION_CHANCE))
                effect_add_random(o_ptr, BIAS_COLD);
            break;
        case EGO_CLOAK_ELEMENTAL_PROTECTION:
            one_ele_resistance(o_ptr);
            do { one_ele_resistance(o_ptr); } while (one_in_(4));
            if (one_in_(7))
                add_flag(o_ptr->art_flags, TR_RES_POIS);
            if (one_in_(ACTIVATION_CHANCE))
                effect_add_random(o_ptr, BIAS_ELEMENTAL);
            break;
        case EGO_CLOAK_BAT:
            o_ptr->to_d -= 6;
            o_ptr->to_h -= 6;
            if (one_in_(3))
                add_flag(o_ptr->art_flags, TR_VULN_LITE);
            if (one_in_(3))
                add_flag(o_ptr->art_flags, TR_DEC_STR);
            break;
        case EGO_CLOAK_FAIRY:
            o_ptr->to_d -= 6;
            o_ptr->to_h -= 6;
            if (one_in_(3))
                add_flag(o_ptr->art_flags, TR_VULN_DARK);
            if (one_in_(3))
                add_flag(o_ptr->art_flags, TR_DEC_STR);
            break;
        case EGO_CLOAK_NAZGUL:
            o_ptr->to_d += 6;
            o_ptr->to_h += 6;
            if (one_in_(6))
                o_ptr->curse_flags |= TRC_PERMA_CURSE;
            if (one_in_(66))
                add_flag(o_ptr->art_flags, TR_IM_COLD);
            while (one_in_(6))
                one_high_resistance(o_ptr);
            if (one_in_(ACTIVATION_CHANCE))
                effect_add_random(o_ptr, BIAS_NECROMANTIC);
            break;
        case EGO_CLOAK_RETRIBUTION:
            if (one_in_(2))
                add_flag(o_ptr->art_flags, TR_SH_FIRE);
            if (one_in_(2))
                add_flag(o_ptr->art_flags, TR_SH_COLD);
            if (one_in_(2))
                add_flag(o_ptr->art_flags, TR_SH_ELEC);
            if (one_in_(7))
                add_flag(o_ptr->art_flags, TR_SH_SHARDS);
            break;
        }
        break;

    case TV_BOOTS:
        if ((!crafting && one_in_(20)) || power > 2)
        {
            _create_artifact(o_ptr, power);
            break;
        }
        while (!done)
        {
            o_ptr->name2 = _get_random_ego(EGO_TYPE_BOOTS);
            done = TRUE;

            switch (o_ptr->name2)
            {
            case EGO_BOOTS_DWARVEN:
                if (o_ptr->sval != SV_PAIR_OF_METAL_SHOD_BOOTS)
                {
                    done = FALSE;
                    break;
                }
                o_ptr->weight = (2 * k_info[o_ptr->k_idx].weight / 3);
                o_ptr->ac = k_info[o_ptr->k_idx].ac + 4;
                if (one_in_(4))
                    add_flag(o_ptr->art_flags, TR_SUST_CON);
                break;
            case EGO_BOOTS_LEVITATION:
            case EGO_BOOTS_ELVENKIND:
            case EGO_BOOTS_FAIRY:
                if (one_in_(2))
                    one_high_resistance(o_ptr);
                break;
            }
        }
        break;
    }
}


/*
 * Hack -- help pick an item type
 */
static bool item_monster_okay(int r_idx)
{
    monster_race *r_ptr = &r_info[r_idx];

    /* No uniques */
    if (r_ptr->flags1 & RF1_UNIQUE) return (FALSE);
    if (r_ptr->flags7 & RF7_KAGE) return (FALSE);
    if (r_ptr->flagsr & RFR_RES_ALL) return (FALSE);
    if (r_ptr->flags7 & RF7_NAZGUL) return (FALSE);
    if (r_ptr->flags1 & RF1_FORCE_DEPTH) return (FALSE);
    if (r_ptr->flags7 & RF7_UNIQUE2) return (FALSE);

    /* Okay */
    return (TRUE);
}

static void _create_lite(object_type *o_ptr, int level, int power, int mode)
{
    bool done = FALSE;

    /* Hack -- Torches and Lanterns -- random fuel */
    if (o_ptr->sval == SV_LITE_TORCH || o_ptr->sval == SV_LITE_LANTERN)
    {
        if (o_ptr->pval > 0) o_ptr->xtra4 = randint1(o_ptr->pval);
        o_ptr->pval = 0;

        if (power == 1 && one_in_(3)) 
            power++;
    }

    if (-1 <= power && power <= 1)
        return;

    if (o_ptr->sval == SV_LITE_FEANOR && (one_in_(20) || power > 2))
    {
        _create_artifact(o_ptr, power);
        return;
    }

    while (!done)
    {
        o_ptr->name2 = _get_random_ego(EGO_TYPE_LITE);
        done = TRUE;
        switch (o_ptr->name2)
        {
        case EGO_LITE_DURATION:
            if (o_ptr->sval == SV_LITE_FEANOR)
                done = FALSE;
            break;
        case EGO_LITE_VALINOR:
            if (o_ptr->sval != SV_LITE_FEANOR)
                done = FALSE;
            else if (one_in_(7))
                add_flag(o_ptr->art_flags, TR_STEALTH);
            break;
        case EGO_LITE_DARKNESS:
            o_ptr->xtra4 = 0;
            break;
        }
    }
}

/*
 * Apply magic to an item known to be "boring"
 *
 * Hack -- note the special code for various items
 */
static void a_m_aux_4(object_type *o_ptr, int level, int power, int mode)
{
    /* Apply magic (good or bad) according to type */
    switch (o_ptr->tval)
    {
        case TV_WHISTLE:
        {
#if 0
            /* Cursed */
            if (power < 0)
            {
                /* Broken */
                o_ptr->ident |= (IDENT_BROKEN);

                /* Cursed */
                o_ptr->curse_flags |= (TRC_CURSED);
            }
#endif
            break;
        }
        case TV_FLASK:
        {
            o_ptr->xtra4 = o_ptr->pval;
            o_ptr->pval = 0;
            break;
        }
        case TV_CAPTURE:
        {
            o_ptr->pval = 0;
            object_aware(o_ptr);
            object_known(o_ptr);
            break;
        }

        case TV_FIGURINE:
        {
            int i = 1;
            int check;

            monster_race *r_ptr;

            /* Pick a random non-unique monster race */
            while (1)
            {
                i = randint1(max_r_idx - 1);

                if (!item_monster_okay(i)) continue;
                if (i == MON_TSUCHINOKO) continue;

                r_ptr = &r_info[i];

                check = (dun_level < r_ptr->level) ? (r_ptr->level - dun_level) : 0;

                /* Ignore dead monsters */
                if (!r_ptr->rarity) continue;

                /* Ignore uncommon monsters */
                if (r_ptr->rarity > 100) continue;

                /* Prefer less out-of-depth monsters */
                if (randint0(check)) continue;

                break;
            }

            o_ptr->pval = i;

            /* Some figurines are cursed */
            if (one_in_(6)) o_ptr->curse_flags |= TRC_CURSED;

            if (cheat_peek)
            {
                msg_format("Figurine of %s, depth +%d%s",

                              r_name + r_ptr->name, check - 1,
                              !object_is_cursed(o_ptr) ? "" : " {cursed}");
            }

            break;
        }

        case TV_CORPSE:
        {
            int i = 1;
            int check = 0;
            int count = 0;

            u32b match = 0;

            monster_race *r_ptr = 0;

            if (o_ptr->sval == SV_SKELETON)
            {
                match = RF9_DROP_SKELETON;
            }
            else if (o_ptr->sval == SV_CORPSE)
            {
                match = RF9_DROP_CORPSE;
            }

            /* Hack -- Remove the monster restriction */
            get_mon_num_prep(item_monster_okay, NULL);

            /* Pick a random non-unique monster race */
            while (1)
            {
                /* This loop is spinning forever at deep levels ... */
                count++;
                if (count > 100) break;

                i = get_mon_num(dun_level);

                r_ptr = &r_info[i];

                check = (dun_level < r_ptr->level) ? (r_ptr->level - dun_level) : 0;

                /* Ignore dead monsters */
                if (!r_ptr->rarity) continue;

                /* Ignore corpseless monsters */
                if (!(r_ptr->flags9 & match)) continue;

                /* Prefer less out-of-depth monsters */
                if (randint0(check)) continue;

                break;
            }

            o_ptr->pval = i;

            if (cheat_peek)
            {
                msg_format("Corpse of %s, depth +%d",

                              r_name + r_ptr->name, check - 1);
            }

            object_aware(o_ptr);
            object_known(o_ptr);
            break;
        }

        case TV_STATUE:
        {
            int i = 1;

            monster_race *r_ptr;

            /* Pick a random monster race */
            while (1)
            {
                i = randint1(max_r_idx - 1);

                r_ptr = &r_info[i];

                /* Ignore dead monsters */
                if (!r_ptr->rarity) continue;

                break;
            }

            o_ptr->pval = i;

            if (cheat_peek)
            {
                msg_format("Statue of %s", r_name + r_ptr->name);

            }
            object_aware(o_ptr);
            object_known(o_ptr);

            break;
        }

        case TV_CHEST:
        {
            byte obj_level = k_info[o_ptr->k_idx].level;

            /* Hack -- skip ruined chests */
            if (obj_level <= 0) break;

            /* Hack -- pick a "difficulty" */
            o_ptr->pval = randint1(obj_level);
            if (o_ptr->sval == SV_CHEST_KANDUME) o_ptr->pval = 6;

            o_ptr->xtra3 = dun_level + 5;

            /* Never exceed "difficulty" of 55 to 59 */
            if (o_ptr->pval > 55) o_ptr->pval = 55 + (byte)randint0(5);

            break;
        }
    }
}


/*
 * Complete the "creation" of an object by applying "magic" to the item
 *
 * This includes not only rolling for random bonuses, but also putting the
 * finishing touches on ego-items and artifacts, giving charges to wands and
 * staffs, giving fuel to lites, and placing traps on chests.
 *
 * In particular, note that "Instant Artifacts", if "created" by an external
 * routine, must pass through this function to complete the actual creation.
 *
 * The base "chance" of the item being "good" increases with the "level"
 * parameter, which is usually derived from the dungeon level, being equal
 * to the level plus 10, up to a maximum of 75. If "good" is true, then
 * the object is guaranteed to be "good". If an object is "good", then
 * the chance that the object will be "great" (ego-item or artifact), also
 * increases with the "level", being equal to half the level, plus 5, up to
 * a maximum of 20. If "great" is true, then the object is guaranteed to be
 * "great". At dungeon level 65 and below, 15/100 objects are "great".
 *
 * If the object is not "good", there is a chance it will be "cursed", and
 * if it is "cursed", there is a chance it will be "broken". These chances
 * are related to the "good" / "great" chances above.
 *
 * Otherwise "normal" rings and amulets will be "good" half the time and
 * "cursed" half the time, unless the ring/amulet is always good or cursed.
 *
 * If "okay" is true, and the object is going to be "great", then there is
 * a chance that an artifact will be created. This is true even if both the
 * "good" and "great" arguments are false. As a total hack, if "great" is
 * true, then the item gets 3 extra "attempts" to become an artifact.
 */
bool apply_magic(object_type *o_ptr, int lev, u32b mode)
{
    int i, rolls, f1, f2, power;

    if (p_ptr->personality == PERS_MUNCHKIN) lev += randint0(p_ptr->lev/2+10);

    /* Maximum "level" for various things */
    if (lev > MAX_DEPTH - 1) lev = MAX_DEPTH - 1;

    /* Base chance of being "good" */
    f1 = lev + 10;

    /* Maximal chance of being "good" */
    if (f1 > d_info[dungeon_type].obj_good) f1 = d_info[dungeon_type].obj_good;

    /* Base chance of being "great" */
    f2 = f1 * 2 / 3;

    /* Maximal chance of being "great" */
    if ((p_ptr->personality != PERS_MUNCHKIN) && (f2 > d_info[dungeon_type].obj_great))
        f2 = d_info[dungeon_type].obj_great;

    /* Temp Hack: It's a bit too hard to find good rings early on. Note we hack after
       calculating f2! */
    if (o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET)
    {
        f1 += 30;
        if (f1 > d_info[dungeon_type].obj_good) f1 = d_info[dungeon_type].obj_good;
    }

    if (p_ptr->good_luck)
    {
        f1 += 5;
        f2 += 2;
    }
    else if(mut_present(MUT_BAD_LUCK))
    {
        f1 -= 5;
        f2 -= 2;
    }

    f1 += virtue_current(VIRTUE_CHANCE) / 50;
    f2 += virtue_current(VIRTUE_CHANCE) / 100;

    /* Assume normal */
    power = 0;

    /* Roll for "good" */
    if ((mode & AM_GOOD) || magik(f1))
    {
        /* Assume "good" */
        power = 1;

        /* Roll for "great" */
        if (no_egos)
        {
        }
        else if ((mode & AM_GREAT) || magik(f2))
        {
            power = 2;

            /* Roll for "special" */
            if (mode & AM_SPECIAL) power = 3;
        }
    }

    /* Roll for "cursed" */
    else if (magik((f1+2)/3))
    {
        /* Assume "cursed" */
        power = -1;

        /* Roll for "broken" */
        if (no_egos)
        {
        }
        else if (magik(f2))
        {
            power = -2;
        }

        /* "Cursed" items become tedious in the late game ... */
        if ( power == -1
          && o_ptr->tval != TV_RING
          && o_ptr->tval != TV_AMULET
          && !object_is_device(o_ptr)
          && randint1(lev) > 10 )
        {
            power = 0;
        }
    }

    if (mode & AM_AVERAGE)
        power = 0;

    /* Apply curse */
    if (mode & AM_CURSED)
    {
        /* Assume 'cursed' */
        if (power > 0)
        {
            power = 0 - power;
        }
        /* Everything else gets more badly cursed */
        else
        {
            power--;
        }
    }

    /* Assume no rolls */
    rolls = 0;

    /* Get one roll if excellent */
    if (power >= 2) rolls = 1;

    /* Hack -- Get four rolls if forced great or special */
    if (mode & (AM_GREAT | AM_SPECIAL)) rolls = 4;

    /* Hack -- Get no rolls if not allowed */
    if ((mode & AM_NO_FIXED_ART) || o_ptr->name1 || o_ptr->name3) rolls = 0;
    if (mode & AM_AVERAGE) rolls = 0;
    if (mode & AM_FORCE_EGO) 
    {
        rolls = 0;
        /* AM_FORCE_EGO is used for granting quest rewards. Ego rings and amulets
           can be achieved with just power 1 ... Indeed, power 2 is often too much! */
        if ((o_ptr->tval == TV_RING || o_ptr->tval == TV_AMULET) && lev < 50)
            power = MAX(1, power);
        else
            power = 2;
    }
    else
        apply_magic_ego = 0;

    /* Roll for artifacts if allowed */
    for (i = 0; i < rolls; i++)
    {
        /* Roll for an artifact */
        if (make_artifact(o_ptr)) break;
        if (p_ptr->good_luck && one_in_(77))
        {
            if (make_artifact(o_ptr)) break;
        }
    }


    /* Hack -- analyze replacement artifacts */
    if (o_ptr->name3)
    {
        artifact_type *a_ptr = &a_info[o_ptr->name3];

        /* Hack -- Mark the artifact as "created" */
        a_ptr->cur_num = 1;

        /* Hack -- Memorize location of artifact in saved floors */
        if (character_dungeon)
            a_ptr->floor_id = p_ptr->floor_id;

        if (cheat_peek) object_mention(o_ptr);
        return TRUE;
    }

    if (object_is_fixed_artifact(o_ptr))
    {
        artifact_type *a_ptr = &a_info[o_ptr->name1];

        /* Hack -- Mark the artifact as "created" */
        a_ptr->cur_num = 1;

        /* Hack -- Memorize location of artifact in saved floors */
        if (character_dungeon)
            a_ptr->floor_id = p_ptr->floor_id;

        /* Extract the other fields */
        o_ptr->pval = a_ptr->pval;
        o_ptr->ac = a_ptr->ac;
        o_ptr->dd = a_ptr->dd;
        o_ptr->ds = a_ptr->ds;
        o_ptr->to_a = a_ptr->to_a;
        o_ptr->to_h = a_ptr->to_h;
        o_ptr->to_d = a_ptr->to_d;
        o_ptr->weight = a_ptr->weight;

        /* Hack -- extract the "broken" flag */
        if (!a_ptr->cost) o_ptr->ident |= (IDENT_BROKEN);

        /* Hack -- extract the "cursed" flag */
        if (a_ptr->gen_flags & TRG_CURSED) o_ptr->curse_flags |= (TRC_CURSED);
        if (a_ptr->gen_flags & TRG_HEAVY_CURSE) o_ptr->curse_flags |= (TRC_HEAVY_CURSE);
        if (a_ptr->gen_flags & TRG_PERMA_CURSE) o_ptr->curse_flags |= (TRC_PERMA_CURSE);
        if (a_ptr->gen_flags & (TRG_RANDOM_CURSE0)) o_ptr->curse_flags |= get_curse(0, o_ptr);
        if (a_ptr->gen_flags & (TRG_RANDOM_CURSE1)) o_ptr->curse_flags |= get_curse(1, o_ptr);
        if (a_ptr->gen_flags & (TRG_RANDOM_CURSE2)) o_ptr->curse_flags |= get_curse(2, o_ptr);


        /* Cheat -- peek at the item */
        if (cheat_peek) object_mention(o_ptr);

        /* Done */
        return TRUE;
    }

    if (o_ptr->art_name)
    {
        return TRUE;
    }


    /* Apply magic */
    switch (o_ptr->tval)
    {
        case TV_DIGGING:
        case TV_HAFTED:
        case TV_BOW:
        case TV_SHOT:
        case TV_ARROW:
        case TV_BOLT:
        {
            /* I couldn't figure out where to put this ... in many ways,
               Harps are more like rings and amulets, so the aux function that
               normally rolls pvals should always be called ... */
            if (o_ptr->tval == TV_BOW && o_ptr->sval == SV_HARP)
                o_ptr->pval = 1 + m_bonus(2, lev);

            if (power) _create_weapon(o_ptr, lev, power, mode);
            break;
        }

        case TV_POLEARM:
        {
            if (power && !(o_ptr->sval == SV_DEATH_SCYTHE)) _create_weapon(o_ptr, lev, power, mode);
            break;
        }

        case TV_SWORD:
        {
            if (object_is_(o_ptr, TV_SWORD, SV_DRAGON_FANG) && !(mode & AM_CRAFTING))
            {
                if (cheat_peek) object_mention(o_ptr);
                dragon_resist(o_ptr);
                if (!one_in_(3)) power = 0;
            }

            if (o_ptr->sval == SV_RUNESWORD) 
            {
                o_ptr->curse_flags |= (TRC_PERMA_CURSE);
            }
            else 
            {
                if (!(o_ptr->sval == SV_DOKUBARI))
                {
                    if (power) _create_weapon(o_ptr, lev, power, mode);
                }
            }
            break;
        }

        case TV_DRAG_ARMOR:
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
        case TV_SHIELD:
        case TV_HELM:
        case TV_CROWN:
        case TV_CLOAK:
        case TV_GLOVES:
        case TV_BOOTS:
            if ( object_is_(o_ptr, TV_CLOAK, SV_ELVEN_CLOAK)
              || object_is_(o_ptr, TV_SOFT_ARMOR, SV_BLACK_CLOTHES) )
            {
                o_ptr->pval = randint1(4);
            }

            if (object_is_dragon_armor(o_ptr) && !(mode & AM_CRAFTING))
            {
                if (cheat_peek) object_mention(o_ptr);
                dragon_resist(o_ptr);
                if (!one_in_(3)) power = 0;
            }
            if (power) _create_armor(o_ptr, lev, power, mode);
            break;
        case TV_RING:
            if (power) _create_ring(o_ptr, lev, power, mode);
            break;
        case TV_AMULET:
            if (power) _create_amulet(o_ptr, lev, power, mode);
            break;
        case TV_LITE:
            _create_lite(o_ptr, lev, power, mode);
            break;
        case TV_WAND:
        case TV_ROD:
        case TV_STAFF:
            return _create_device(o_ptr, lev, power, mode);
        default:
        {
            a_m_aux_4(o_ptr, lev, power, mode);
            break;
        }
    }

    if ((o_ptr->tval == TV_SOFT_ARMOR) &&
        (o_ptr->sval == SV_ABUNAI_MIZUGI) &&
        (p_ptr->personality == PERS_SEXY || demigod_is_(DEMIGOD_APHRODITE)))
    {
        o_ptr->pval = 3;
        add_flag(o_ptr->art_flags, TR_STR);
        add_flag(o_ptr->art_flags, TR_INT);
        add_flag(o_ptr->art_flags, TR_WIS);
        add_flag(o_ptr->art_flags, TR_DEX);
        add_flag(o_ptr->art_flags, TR_CON);
        add_flag(o_ptr->art_flags, TR_CHR);
    }

    /* Hack -- analyze ego-items */
    if (object_is_ego(o_ptr))
    {
        ego_item_type *e_ptr = &e_info[o_ptr->name2];

        if (!store_hack)
            e_ptr->counts.generated++;

        /* Hack -- acquire "cursed" flag */
        if (e_ptr->gen_flags & TRG_CURSED) o_ptr->curse_flags |= (TRC_CURSED);
        if (e_ptr->gen_flags & TRG_HEAVY_CURSE) o_ptr->curse_flags |= (TRC_HEAVY_CURSE);
        if (e_ptr->gen_flags & TRG_PERMA_CURSE) o_ptr->curse_flags |= (TRC_PERMA_CURSE);
        if (e_ptr->gen_flags & (TRG_RANDOM_CURSE0)) o_ptr->curse_flags |= get_curse(0, o_ptr);
        if (e_ptr->gen_flags & (TRG_RANDOM_CURSE1)) o_ptr->curse_flags |= get_curse(1, o_ptr);
        if (e_ptr->gen_flags & (TRG_RANDOM_CURSE2)) o_ptr->curse_flags |= get_curse(2, o_ptr);

        if (e_ptr->gen_flags & (TRG_ONE_SUSTAIN)) one_sustain(o_ptr);
        if (e_ptr->gen_flags & (TRG_XTRA_POWER)) one_ability(o_ptr);
        if (e_ptr->gen_flags & (TRG_XTRA_H_RES))
        {
            one_high_resistance(o_ptr);
            if (randint1(object_level) > 60)
                one_high_resistance(o_ptr);
        }
        if (e_ptr->gen_flags & (TRG_XTRA_E_RES)) one_ele_resistance(o_ptr);
        if (e_ptr->gen_flags & (TRG_XTRA_D_RES)) one_dragon_ele_resistance(o_ptr);
        if (e_ptr->gen_flags & (TRG_XTRA_L_RES)) one_lordly_high_resistance(o_ptr);
        if (e_ptr->gen_flags & (TRG_XTRA_RES)) one_resistance(o_ptr);

        /* Hack -- obtain bonuses */
        if (e_ptr->max_to_h)
        {
            if (e_ptr->max_to_h < 0)
                o_ptr->to_h -= randint1(-e_ptr->max_to_h);
            else 
                o_ptr->to_h += randint1(e_ptr->max_to_h);
        }
        if (e_ptr->max_to_d)
        {
            if (e_ptr->max_to_d < 0)
                o_ptr->to_d -= randint1(-e_ptr->max_to_d);
            else 
                o_ptr->to_d += randint1(e_ptr->max_to_d);
        }
        if (e_ptr->max_to_a)
        {
            if (e_ptr->max_to_a < 0)
                o_ptr->to_a -= randint1(-e_ptr->max_to_a);
            else 
                o_ptr->to_a += randint1(e_ptr->max_to_a);
        }

        /* Hack -- obtain pval */
        if (e_ptr->max_pval)
        {
            if ((o_ptr->name2 == EGO_WEAPON_CRUSADE) && (have_flag(o_ptr->art_flags, TR_BLOWS)))
            {
                if (o_ptr->dd*o_ptr->ds > 30)
                {
                    remove_flag(o_ptr->art_flags, TR_BLOWS);
                    /* As far as I can tell, pval is first assigned for Holy Avengers
                        as the terminal else clause of this (outer) if, which is missed
                        by having picked up extra blows ... */
                    o_ptr->pval = randint1(e_ptr->max_pval);
                }
                else
                {
                    o_ptr->pval = randint1(2);
                    if ((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_HAYABUSA))
                        o_ptr->pval += randint1(2);
                    if ((lev > 60) && one_in_(3) && ((o_ptr->dd*(o_ptr->ds+1)) < 15)) o_ptr->pval += randint1(2);
                }
            }
            else if (o_ptr->name2 == EGO_WEAPON_EXTRA_ATTACKS)
            {
                o_ptr->pval = randint1(e_ptr->max_pval*lev/100+1);
                if (o_ptr->pval > 6) o_ptr->pval = 6;
                if (o_ptr->pval == 6 && !one_in_(o_ptr->dd * o_ptr->ds / 2)) o_ptr->pval = 5;
                if ((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_HAYABUSA))
                    o_ptr->pval += randint1(2);

                if (o_ptr->dd*o_ptr->ds > 30)
                    o_ptr->pval = MAX(o_ptr->pval, 3);
            }
            else if ( o_ptr->name2 == EGO_CLOAK_BAT 
                   || o_ptr->name2 == EGO_CLOAK_FAIRY 
                   || o_ptr->name2 == EGO_CLOAK_COWARDICE
                   || o_ptr->name2 == EGO_CLOAK_AMAN )
            {
                o_ptr->pval = randint1(e_ptr->max_pval);
                if (o_ptr->sval == SV_ELVEN_CLOAK) o_ptr->pval += randint0(2);
            }
            else if (o_ptr->name2 == EGO_GLOVES_BERSERKER)
            {
                o_ptr->pval = randint1(2);
                if (one_in_(15))
                    o_ptr->pval++;
            }
            else
            {
                o_ptr->pval += randint1(e_ptr->max_pval);
            }
        }
        if (o_ptr->name2 == EGO_BOOTS_SPEED)
        {
            o_ptr->pval = 1 + m_bonus(9, object_level);
        }
        if (o_ptr->name2 == EGO_BOOTS_FEANOR)
        {
            o_ptr->pval = 6 + m_bonus(9, object_level);
        }
        if (have_flag(o_ptr->art_flags, TR_DEVICE_POWER) && o_ptr->pval >= 3)
        {
            o_ptr->pval = 2;
            if (one_in_(30)) o_ptr->pval++;
        }

        if ((o_ptr->tval == TV_SWORD) && (o_ptr->sval == SV_HAYABUSA) && (o_ptr->pval > 2) && (o_ptr->name2 != EGO_WEAPON_EXTRA_ATTACKS))
            o_ptr->pval = 2;

        /* Cursed Egos: Make sure to do this last to avoid nonsensical combinations of 
           good and bad flags (e.g. resist fire and vulnerable to fire) */
        if (power == -2)
        {
            curse_object(o_ptr);
            if (!o_ptr->pval && have_pval_flags(o_ptr->art_flags))
                o_ptr->pval = randint1(3);
        }

        if (cheat_peek) object_mention(o_ptr);
        return TRUE;
    }

    if (o_ptr->art_name)
    {
        if (!store_hack)
            stats_rand_art_counts.generated++;
    }

    /* Examine real objects */
    if (o_ptr->k_idx)
    {
        object_kind *k_ptr = &k_info[o_ptr->k_idx];

        /* Hack -- acquire "broken" flag */
        if (!k_info[o_ptr->k_idx].cost) o_ptr->ident |= (IDENT_BROKEN);

        /* Hack -- acquire "cursed" flag */
        if (k_ptr->gen_flags & (TRG_CURSED)) o_ptr->curse_flags |= (TRC_CURSED);
        if (k_ptr->gen_flags & (TRG_HEAVY_CURSE)) o_ptr->curse_flags |= TRC_HEAVY_CURSE;
        if (k_ptr->gen_flags & (TRG_PERMA_CURSE)) o_ptr->curse_flags |= TRC_PERMA_CURSE;
        if (k_ptr->gen_flags & (TRG_RANDOM_CURSE0)) o_ptr->curse_flags |= get_curse(0, o_ptr);
        if (k_ptr->gen_flags & (TRG_RANDOM_CURSE1)) o_ptr->curse_flags |= get_curse(1, o_ptr);
        if (k_ptr->gen_flags & (TRG_RANDOM_CURSE2)) o_ptr->curse_flags |= get_curse(2, o_ptr);
    }
    return TRUE;
}

static bool _is_favorite_weapon(int tval, int sval)
{
    if (p_ptr->pclass != CLASS_ARCHER)
    {
        object_type forge;
        int         k_idx = lookup_kind(tval, sval);

        object_prep(&forge, k_idx);
        return object_is_favorite(&forge);
    }
    return FALSE;
}

static bool _is_device_class(void)
{
    int class_idx = p_ptr->pclass;

    if (class_idx == CLASS_MONSTER)
        return get_race()->pseudo_class_idx;

    switch (class_idx)
    {
    case CLASS_ARCHAEOLOGIST:
    case CLASS_BLOOD_MAGE:
    case CLASS_HIGH_MAGE:
    case CLASS_MAGE:
    case CLASS_MAGIC_EATER:
    case CLASS_MIRROR_MASTER:
    case CLASS_NECROMANCER:
    case CLASS_ROGUE:
    case CLASS_SORCERER:
        return TRUE;
    }
    /* Note: Devicemasters only want their speciality, which is checked below. */
    return FALSE;
}

static bool kind_is_tailored(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    switch (k_ptr->tval)
    {
    case TV_SHIELD:
        return equip_can_wield_kind(k_ptr->tval, k_ptr->sval) 
            && p_ptr->pclass != CLASS_NINJA
            && p_ptr->pclass != CLASS_MAULER
            && p_ptr->pclass != CLASS_DUELIST;

    case TV_HARD_ARMOR:
    case TV_SOFT_ARMOR:
    case TV_DRAG_ARMOR:
        if ( p_ptr->pclass == CLASS_MONK
          || p_ptr->pclass == CLASS_FORCETRAINER
          || p_ptr->pclass == CLASS_MYSTIC 
          || p_ptr->pclass == CLASS_DUELIST
          || p_ptr->pclass == CLASS_SCOUT
          || p_ptr->pclass == CLASS_NINJA )
        {
            return k_ptr->weight <= 200;
        }
        return equip_can_wield_kind(k_ptr->tval, k_ptr->sval);

    case TV_CLOAK:
    case TV_BOOTS:
    case TV_GLOVES:
    case TV_HELM:
    case TV_CROWN:
    case TV_BOW:
        return equip_can_wield_kind(k_ptr->tval, k_ptr->sval);

    case TV_RING:
    case TV_AMULET:
        if (p_ptr->prace == RACE_MON_RING) return TRUE;
        else return equip_can_wield_kind(k_ptr->tval, k_ptr->sval);

    case TV_SWORD:
    case TV_HAFTED:
    case TV_POLEARM:
    case TV_DIGGING:
        return equip_can_wield_kind(k_ptr->tval, k_ptr->sval)
            && _is_favorite_weapon(k_ptr->tval, k_ptr->sval);

    case TV_SHOT:
        /*return equip_can_wield_kind(TV_BOW, SV_SLING);*/
        return FALSE;

    case TV_BOLT:
        /*return equip_can_wield_kind(TV_BOW, SV_LIGHT_XBOW);*/
        return FALSE;

    case TV_ARROW:
        /*return equip_can_wield_kind(TV_BOW, SV_LONG_BOW);*/
        return FALSE;

    case TV_LIFE_BOOK:
    case TV_SORCERY_BOOK:
    case TV_NATURE_BOOK:
    case TV_CHAOS_BOOK:
    case TV_DEATH_BOOK:
    case TV_TRUMP_BOOK:
    case TV_CRAFT_BOOK:
    case TV_DAEMON_BOOK:
    case TV_CRUSADE_BOOK:
    case TV_NECROMANCY_BOOK:
    case TV_ARMAGEDDON_BOOK:
    case TV_MUSIC_BOOK:
    case TV_HISSATSU_BOOK:
    case TV_HEX_BOOK:
    case TV_RAGE_BOOK:
    case TV_BURGLARY_BOOK:
        return check_book_realm(k_ptr->tval, k_ptr->sval)
            && k_ptr->sval >= SV_BOOK_MIN_GOOD;

    case TV_WAND:
        return devicemaster_is_(DEVICEMASTER_WANDS)
            || _is_device_class();

    case TV_ROD:
        return devicemaster_is_(DEVICEMASTER_RODS)
            || _is_device_class();

    case TV_STAFF:
        return devicemaster_is_(DEVICEMASTER_STAVES)
            || _is_device_class();

    case TV_POTION:
        return devicemaster_is_(DEVICEMASTER_POTIONS);

    case TV_SCROLL:
        return devicemaster_is_(DEVICEMASTER_SCROLLS);
    }

    return FALSE;
}

static bool _drop_tailored = FALSE;

bool kind_is_great(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    if (_drop_tailored && !kind_is_tailored(k_idx))
        return FALSE;

    /* Analyze the item type */
    switch (k_ptr->tval)
    {
        /* Armor -- Good unless damaged */
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
        case TV_DRAG_ARMOR:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        {
            if (k_ptr->to_a < 0) return (FALSE);
            return (TRUE);
        }

        /* Weapons -- Good unless damaged */
        case TV_BOW:
        case TV_SWORD:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_DIGGING:
        {
            if (k_ptr->to_h < 0) return (FALSE);
            if (k_ptr->to_d < 0) return (FALSE);
            return (TRUE);
        }

        /* Ammo -- Arrows/Bolts are good */
        case TV_BOLT:
        case TV_ARROW:
        {
            return (TRUE);
        }

        /* Books -- High level books are good (except Arcane books) */
        case TV_LIFE_BOOK:
        case TV_SORCERY_BOOK:
        case TV_NATURE_BOOK:
        case TV_CHAOS_BOOK:
        case TV_DEATH_BOOK:
        case TV_TRUMP_BOOK:
        case TV_CRAFT_BOOK:
        case TV_DAEMON_BOOK:
        case TV_CRUSADE_BOOK:
        case TV_NECROMANCY_BOOK:
        case TV_ARMAGEDDON_BOOK:
        case TV_MUSIC_BOOK:
        case TV_HISSATSU_BOOK:
        case TV_HEX_BOOK:
        case TV_RAGE_BOOK:
        case TV_BURGLARY_BOOK:
        {
            if (k_ptr->sval == SV_BOOK_MIN_GOOD) return TRUE; /* Third Spellbooks: I want ?Acquirement to grant these! */
            if (k_ptr->sval >= SV_BOOK_MIN_GOOD + 1) return TRUE;   /* Fourth Spellbooks */
            return (FALSE);
        }
        case TV_POTION:
        {
            if (k_ptr->sval == SV_POTION_LIFE) return TRUE;
            if (k_ptr->sval == SV_POTION_STAR_HEALING) return TRUE;
            if (k_ptr->sval == SV_POTION_AUGMENTATION) return TRUE;
            return FALSE;
        }
        case TV_SCROLL:
        {
            if (k_ptr->sval == SV_SCROLL_ACQUIREMENT) return TRUE;
            if (k_ptr->sval == SV_SCROLL_STAR_ACQUIREMENT) return TRUE;
            if (k_ptr->sval == SV_SCROLL_STAR_DESTRUCTION) return TRUE;
            if (k_ptr->sval == SV_SCROLL_GENOCIDE) return TRUE;
            if (k_ptr->sval == SV_SCROLL_MASS_GENOCIDE) return TRUE;
            if (k_ptr->sval == SV_SCROLL_ARTIFACT) return TRUE;
            if (k_ptr->sval == SV_SCROLL_MANA) return TRUE;
            return FALSE;
        }
        case TV_WAND:
        case TV_STAFF:
        case TV_ROD:
            return TRUE;

        case TV_RING:
        case TV_AMULET:
            return TRUE;
    }

    /* Assume not good */
    return (FALSE);
}

bool kind_is_good(int k_idx)
{
    object_kind *k_ptr = &k_info[k_idx];

    if (_drop_tailored && !kind_is_tailored(k_idx))
        return FALSE;

    /* Analyze the item type */
    switch (k_ptr->tval)
    {
        /* Armor -- Good unless damaged */
        case TV_HARD_ARMOR:
        case TV_SOFT_ARMOR:
        case TV_DRAG_ARMOR:
        case TV_SHIELD:
        case TV_CLOAK:
        case TV_BOOTS:
        case TV_GLOVES:
        case TV_HELM:
        case TV_CROWN:
        {
            if (k_ptr->to_a < 0) return (FALSE);
            return (TRUE);
        }

        /* Weapons -- Good unless damaged */
        case TV_BOW:
        case TV_SWORD:
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_DIGGING:
        {
            if (k_ptr->to_h < 0) return (FALSE);
            if (k_ptr->to_d < 0) return (FALSE);
            return (TRUE);
        }

        /* Ammo -- Arrows/Bolts are good */
        case TV_BOLT:
        case TV_ARROW:
        {
            return (TRUE);
        }

        /* Books -- High level books are good (except Arcane books) */
        case TV_LIFE_BOOK:
        case TV_SORCERY_BOOK:
        case TV_NATURE_BOOK:
        case TV_CHAOS_BOOK:
        case TV_DEATH_BOOK:
        case TV_TRUMP_BOOK:
        case TV_CRAFT_BOOK:
        case TV_DAEMON_BOOK:
        case TV_CRUSADE_BOOK:
        case TV_NECROMANCY_BOOK:
        case TV_ARMAGEDDON_BOOK:
        case TV_MUSIC_BOOK:
        case TV_HISSATSU_BOOK:
        case TV_HEX_BOOK:
        case TV_RAGE_BOOK:
        case TV_BURGLARY_BOOK:
        {
            if (k_ptr->sval == SV_BOOK_MIN_GOOD) return TRUE; /* Third Spellbooks */
            if (k_ptr->sval >= SV_BOOK_MIN_GOOD + 1) return TRUE;   /* Fourth Spellbooks */
            return (FALSE);
        }
        case TV_POTION:
        {
            switch (k_ptr->sval)
            {
            case SV_POTION_RESISTANCE: return TRUE;
            case SV_POTION_LIFE: return TRUE;
            case SV_POTION_HEALING: return TRUE;
            case SV_POTION_STAR_HEALING: return TRUE;
            case SV_POTION_AUGMENTATION: return TRUE;
            case SV_POTION_INC_STR:
            case SV_POTION_INC_INT:
            case SV_POTION_INC_WIS:
            case SV_POTION_INC_DEX:
            case SV_POTION_INC_CON:
            case SV_POTION_INC_CHR: return TRUE;
            }
            return FALSE;
        }
        case TV_SCROLL:
        {
            if (k_ptr->sval == SV_SCROLL_ACQUIREMENT) return TRUE;
            if (k_ptr->sval == SV_SCROLL_FOREST_CREATION) return TRUE;
            if (k_ptr->sval == SV_SCROLL_WALL_CREATION) return TRUE;
            if (k_ptr->sval == SV_SCROLL_VENGEANCE) return TRUE;
            if (k_ptr->sval == SV_SCROLL_STAR_ACQUIREMENT) return TRUE;
            if (k_ptr->sval == SV_SCROLL_STAR_DESTRUCTION) return TRUE;
            if (k_ptr->sval == SV_SCROLL_GENOCIDE) return TRUE;
            if (k_ptr->sval == SV_SCROLL_MASS_GENOCIDE) return TRUE;
            /*if (k_ptr->sval == SV_SCROLL_ARTIFACT) return TRUE;*/
            if (k_ptr->sval == SV_SCROLL_FIRE) return TRUE;
            if (k_ptr->sval == SV_SCROLL_ICE) return TRUE;
            if (k_ptr->sval == SV_SCROLL_CHAOS) return TRUE;
            if (k_ptr->sval == SV_SCROLL_MANA) return TRUE;
            return FALSE;
        }
        case TV_WAND:
        case TV_STAFF:
        case TV_ROD:
            return TRUE;

        case TV_RING:
        case TV_AMULET:
            return TRUE;
    }

    /* Assume not good */
    return (FALSE);
}

typedef bool (*_kind_p)(int k_idx);
static _kind_p _kind_hook1;
static _kind_p _kind_hook2;
static bool _kind_hook(int k_idx) { 
    if (_kind_hook1 && !_kind_hook1(k_idx))
        return FALSE;
    if (_kind_hook2 && !_kind_hook2(k_idx))
        return FALSE;
    return TRUE;
}
bool kind_is_device(int k_idx) { 
    switch (k_info[k_idx].tval)
    {
    case TV_POTION: case TV_SCROLL: case TV_WAND: case TV_ROD: case TV_STAFF:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_potion(int k_idx) { 
    switch (k_info[k_idx].tval)
    {
    case TV_POTION:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_scroll(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_SCROLL:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_wand(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_WAND:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_rod(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_ROD:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_staff(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_STAFF:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_potion_scroll(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_POTION:
    case TV_SCROLL:
        return TRUE;
    }
    return FALSE;
}
bool kind_is_wand_rod_staff(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_WAND:
    case TV_ROD:
    case TV_STAFF:
        return TRUE;
    }
    return FALSE;
}
bool kind_is_jewelry(int k_idx) { 
    switch (k_info[k_idx].tval)
    {
    case TV_RING: case TV_AMULET:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_ring(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_RING:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_amulet(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_AMULET:
        return TRUE;
    }
    return FALSE;
}
bool kind_is_book(int k_idx) {
    if (TV_LIFE_BOOK <= k_info[k_idx].tval && k_info[k_idx].tval <= TV_BURGLARY_BOOK)
        return TRUE;
    return FALSE;
}
bool kind_is_body_armor(int k_idx) { 
    switch (k_info[k_idx].tval)
    {
    case TV_SOFT_ARMOR: case TV_HARD_ARMOR: case TV_DRAG_ARMOR:
    case TV_SHIELD:
        return TRUE;
    }
    return FALSE;
}
bool kind_is_other_armor(int k_idx) { 
    switch (k_info[k_idx].tval)
    {
    case TV_BOOTS: case TV_GLOVES: 
    case TV_HELM: case TV_CROWN: case TV_CLOAK:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_helm_cloak(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_HELM: case TV_CROWN: case TV_CLOAK:
        return TRUE;
    }
    return FALSE;
}
bool kind_is_helm(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_HELM: case TV_CROWN:
        return TRUE;
    }
    return FALSE;
}
static bool _kind_is_boots(int k_idx) {
    switch (k_info[k_idx].tval)
    {
    case TV_BOOTS:
        return TRUE;
    }
    return FALSE;
}
bool kind_is_armor(int k_idx) {
    if (TV_ARMOR_BEGIN <= k_info[k_idx].tval && k_info[k_idx].tval <= TV_ARMOR_END)
        return TRUE;
    return FALSE;
}
bool kind_is_weapon(int k_idx) { 
    if (TV_DIGGING <= k_info[k_idx].tval && k_info[k_idx].tval <= TV_WEAPON_END)
        return TRUE;
    return FALSE;
}
static bool _kind_is_whip(int k_idx) {
    if (k_info[k_idx].tval == TV_HAFTED && k_info[k_idx].sval == SV_WHIP)
        return TRUE;
    return FALSE;
}
static bool _kind_is_lance(int k_idx) {
    if (k_info[k_idx].tval == TV_POLEARM && (k_info[k_idx].sval == SV_LANCE || k_info[k_idx].sval == SV_HEAVY_LANCE))
        return TRUE;
    return FALSE;
}
static bool _kind_is_bow(int k_idx) {
    if (k_info[k_idx].tval == TV_BOW && k_info[k_idx].sval != SV_HARP) /* Assume tailored Archer reward, and a harp is just insulting! */
        return TRUE;
    return FALSE;
}
bool kind_is_bow_ammo(int k_idx) {
    if (k_info[k_idx].tval == TV_BOW)
        return TRUE;
    if (TV_MISSILE_BEGIN <= k_info[k_idx].tval && k_info[k_idx].tval <= TV_MISSILE_END)
        return TRUE;
    return FALSE;
}
bool kind_is_misc(int k_idx) { 
    switch (k_info[k_idx].tval)
    {
    case TV_SKELETON: case TV_BOTTLE: case TV_JUNK: case TV_WHISTLE:
    case TV_SPIKE: case TV_CHEST: case TV_FIGURINE: case TV_STATUE:
    case TV_CAPTURE: case TV_LITE: case TV_FOOD: case TV_FLASK:
        return TRUE;
    }
    return FALSE;
}
typedef struct {
    _kind_p hook;
    int     base;
    int     good;
    int     great;
} _kind_alloc_entry;
static _kind_alloc_entry _kind_alloc_table[] = {
    { kind_is_weapon,          200,    0,    0 },
    { kind_is_body_armor,      190,    0,    0 },
    { kind_is_other_armor,     210,    0,    0 },
    { kind_is_wand_rod_staff,   90,  -40,  -60 },
    { _kind_is_potion_scroll,  100,  -50,  -90 },
    { kind_is_bow_ammo,         70,    0,    0 },
    { kind_is_book,             50,    0,    0 },
    { kind_is_jewelry,          40,    0,    0 },
    { kind_is_misc,             50,  -50,  -50 },
    { NULL, 0}
};
static int _kind_alloc_weight(_kind_alloc_entry *entry, u32b mode)
{
    int w = 0;
    w = entry->base;
    if (mode & AM_GREAT)
        w += entry->great;
    else if (mode & AM_GOOD)
        w += entry->good;

    if (p_ptr->prace == RACE_MON_RING && entry->hook == kind_is_jewelry)
        w = w * 2;

    return MAX(0, w);
}
static _kind_p _choose_obj_kind(u32b mode)
{
    int  i;
    int  tot = 0;

    _kind_hook1 = NULL;
    _kind_hook2 = NULL;

    if (mode & AM_GREAT)
        _kind_hook2 = kind_is_great;
    else if (mode & AM_GOOD)
        _kind_hook2 = kind_is_good;
    else if (_drop_tailored)
        _kind_hook2 = kind_is_tailored;

    /* Circumvent normal object kind frequencies for good, great and tailored drops.
     * The goal here is, for example, to allow mages to find adequate spellbooks without
     * drowning other characters with useless junk. */
    if (_drop_tailored)
    {
        switch (p_ptr->pclass)
        {
        case CLASS_ARCHAEOLOGIST:
            if (one_in_(5))
                _kind_hook1 = _kind_is_whip; /* Diggers swamp out whips ... */
            else if (one_in_(5))
                _kind_hook1 = kind_is_weapon;
            else if (one_in_(5))
                _kind_hook1 = kind_is_wand_rod_staff;
            break;
        case CLASS_ARCHER:
        case CLASS_SNIPER:
            if (one_in_(5))
                _kind_hook1 = _kind_is_bow;
            break;
        case CLASS_DEVICEMASTER:
            if (one_in_(5))
            {
                switch (p_ptr->psubclass)
                {
                case DEVICEMASTER_POTIONS:
                    _kind_hook1 = _kind_is_potion;
                    break;
                case DEVICEMASTER_SCROLLS:
                    _kind_hook1 = _kind_is_scroll;
                    break;
                case DEVICEMASTER_RODS:
                    _kind_hook1 = _kind_is_rod;
                    break;
                case DEVICEMASTER_WANDS:
                    _kind_hook1 = _kind_is_wand;
                    break;
                case DEVICEMASTER_STAVES:
                    _kind_hook1 = _kind_is_staff;
                    break;
                }
            }
            break;
        case CLASS_MAGIC_EATER:
            if (one_in_(5))
                _kind_hook1 = kind_is_device;
            break;
        case CLASS_RAGE_MAGE:
            if (one_in_(3))
                _kind_hook1 = kind_is_book;
            break;
        case CLASS_CAVALRY:
        case CLASS_BEASTMASTER:
            if (one_in_(7))
                _kind_hook1 = _kind_is_lance;
            break;

        case CLASS_MONSTER:
            switch (p_ptr->prace)
            {
            case RACE_MON_BEHOLDER:
                if (one_in_(5))
                    _kind_hook1 = _kind_is_ring;
                else if (one_in_(7))
                    _kind_hook1 = kind_is_helm;
                break;
            case RACE_MON_CENTIPEDE:
                if (one_in_(7))
                    _kind_hook1 = _kind_is_boots;
                break;
            case RACE_MON_DRAGON:
                if (one_in_(5))
                    _kind_hook1 = _kind_is_ring;
                else if (one_in_(7))
                    _kind_hook1 = _kind_is_helm_cloak;
                break;
            case RACE_MON_HYDRA:
                if (one_in_(5))
                    _kind_hook1 = _kind_is_amulet;
                else if (one_in_(7))
                    _kind_hook1 = kind_is_helm;
                break;
            }
            break;
        }
        if (!_kind_hook1 && mut_present(MUT_DRACONIAN_METAMORPHOSIS))
        {
            if (one_in_(5))
                _kind_hook1 = _kind_is_ring;
            else if (one_in_(7))
                _kind_hook1 = _kind_is_helm_cloak;
        }
        if (!_kind_hook1)
        {
            if (is_magic(p_ptr->realm1) && one_in_(5))
                _kind_hook1 = kind_is_book;
            else if (_is_device_class() && one_in_(7))
                _kind_hook1 = kind_is_wand_rod_staff;
        }
    }

    /* Otherwise, pick the kind of drop using the allocation table defined above.
     * This allows us to design, say, 3% of drops to be jewelry, 4% potions, or
     * whatever. */
    if (!_kind_hook1)
    {
        for (i = 0; ; i++)
        {
            if (!_kind_alloc_table[i].hook) break;
            tot += _kind_alloc_weight(&_kind_alloc_table[i], mode);
        }

        if (tot > 0)
        {
            int j = randint0(tot);

            for (i = 0; ; i++)
            {
                if (!_kind_alloc_table[i].hook) break;
                j -= _kind_alloc_weight(&_kind_alloc_table[i], mode);
                if (j < 0)
                {
                    _kind_hook1 = _kind_alloc_table[i].hook;
                    break;
                }
            }
        }
    }
    return _kind_hook;
}

void choose_obj_kind(int mode)
{
    if (!get_obj_num_hook)
        get_obj_num_hook = _choose_obj_kind(mode);
}

/*
 * Attempt to make an object (normal or good/great)
 *
 * This routine plays nasty games to generate the "special artifacts".
 *
 * This routine uses "object_level" for the "generation level".
 *
 * We assume that the given object has been "wiped".
 */
bool make_object(object_type *j_ptr, u32b mode)
{
    int prob, base;
    byte obj_level;

    /* Chance of "special object" */
    prob = ((mode & AM_GOOD) ? 10 : 1000);

    /* Base level for the object */
    base = ((mode & AM_GOOD) ? (object_level + 10) : object_level);

    /* Generate a special object, or a normal object */
    if (!one_in_(prob) || !make_artifact_special(j_ptr))
    {
        int k_idx;
        int max_attempts = 1;
        int attempt = 1;

        _drop_tailored = FALSE;
        if (mode & AM_TAILORED)
        {
            _drop_tailored = TRUE;
            max_attempts = 1000; /* Tailored drops can fail for certain _choose_obj_kind()s */
        }

        for (;;)
        {
            if (!get_obj_num_hook)
                get_obj_num_hook = _choose_obj_kind(mode);

            if (get_obj_num_hook) 
                get_obj_num_prep();

            k_idx = get_obj_num(base);

            if (get_obj_num_hook)
            {
                get_obj_num_hook = NULL;
                get_obj_num_prep();
            }

            if (k_idx)
                break;

            attempt++;
            if (attempt > max_attempts)
                break;
        }

        /* Handle failure */
        if (!k_idx) 
            return (FALSE);

        /* Prepare the object */
        object_prep(j_ptr, k_idx);
    }

    /* Apply magic (allow artifacts) */
    if (!apply_magic(j_ptr, object_level, mode))
        return FALSE;

    /* Note: It is important to do this *after* apply_magic rather than in, say, 
       object_prep() since artifacts should never spawn multiple copies. Ego ammo
       should, but other egos (e.g. lights) should not. */
    mass_produce(j_ptr);

    obj_level = k_info[j_ptr->k_idx].level;
    if (object_is_fixed_artifact(j_ptr)) obj_level = a_info[j_ptr->name1].level;
    if (j_ptr->name3) obj_level = a_info[j_ptr->name3].level;

    /* Notice "okay" out-of-depth objects */
    if (!object_is_cursed(j_ptr) && !object_is_broken(j_ptr) &&
        (obj_level > dun_level))
    {
        /* Cheat -- peek at items */
        if (cheat_peek) object_mention(j_ptr);
    }

    /* Success */
    return (TRUE);
}


/*
 * Attempt to place an object (normal or good/great) at the given location.
 *
 * This routine plays nasty games to generate the "special artifacts".
 *
 * This routine uses "object_level" for the "generation level".
 *
 * This routine requires a clean floor grid destination.
 */
void place_object(int y, int x, u32b mode)
{
    s16b o_idx;

    /* Acquire grid */
    cave_type *c_ptr = &cave[y][x];

    object_type forge;
    object_type *q_ptr;


    /* Paranoia -- check bounds */
    if (!in_bounds(y, x)) return;

    /* Require floor space */
    if (!cave_drop_bold(y, x)) return;

    /* Avoid stacking on other objects */
    if (c_ptr->o_idx) return;


    /* Get local object */
    q_ptr = &forge;

    /* Wipe the object */
    object_wipe(q_ptr);

    /* Make an object (if possible) */
    if (!make_object(q_ptr, mode)) return;


    /* Make an object */
    o_idx = o_pop();

    /* Success */
    if (o_idx)
    {
        object_type *o_ptr;

        /* Acquire object */
        o_ptr = &o_list[o_idx];

        /* Structure Copy */
        object_copy(o_ptr, q_ptr);

        /* Location */
        o_ptr->iy = y;
        o_ptr->ix = x;

        /* Build a stack */
        o_ptr->next_o_idx = c_ptr->o_idx;

        /* Place the object */
        c_ptr->o_idx = o_idx;

        /* Notice */
        note_spot(y, x);

        /* Redraw */
        lite_spot(y, x);
    }
    else
    {
        /* Hack -- Preserve artifacts */
        if (object_is_fixed_artifact(q_ptr))
        {
            a_info[q_ptr->name1].cur_num = 0;
        }
        if (q_ptr->name3)
            a_info[q_ptr->name3].cur_num = 0;
    }
}


/*
 * Make a treasure object
 *
 * The location must be a legal, clean, floor grid.
 */
bool make_gold(object_type *j_ptr)
{
    int i;

    s32b base;


    /* Hack -- Pick a Treasure variety */
    i = ((randint1(object_level + 2) + 2) / 2) - 1;

    /* Apply "extra" magic */
    if (one_in_(GREAT_OBJ))
    {
        i += randint1(object_level + 1);
    }

    /* Hack -- Creeping Coins only generate "themselves" */
    if (coin_type) i = coin_type;

    /* Do not create "illegal" Treasure Types */
    if (i >= MAX_GOLD) i = MAX_GOLD - 1;

    /* Prepare a gold object */
    object_prep(j_ptr, OBJ_GOLD_LIST + i);

    /* Hack -- Base coin cost */
    base = k_info[OBJ_GOLD_LIST+i].cost;

    /* Determine how much the treasure is "worth" */
    j_ptr->pval = (base + (8 * randint1(base)) + randint1(8));

    if (no_selling)
    {
        j_ptr->pval += j_ptr->pval / 2;
        j_ptr->pval += j_ptr->pval * 2 * object_level / 100;
    }

    j_ptr->pval = j_ptr->pval * (625 - virtue_current(VIRTUE_SACRIFICE)) / 625;

    /* Success */
    return (TRUE);
}


/*
 * Places a treasure (Gold or Gems) at given location
 *
 * The location must be a legal, clean, floor grid.
 */
void place_gold(int y, int x)
{
    s16b o_idx;

    /* Acquire grid */
    cave_type *c_ptr = &cave[y][x];


    object_type forge;
    object_type *q_ptr;


    /* Paranoia -- check bounds */
    if (!in_bounds(y, x)) return;

    /* Require floor space */
    if (!cave_drop_bold(y, x)) return;

    /* Avoid stacking on other objects */
    if (c_ptr->o_idx) return;


    /* Get local object */
    q_ptr = &forge;

    /* Wipe the object */
    object_wipe(q_ptr);

    /* Make some gold */
    if (!make_gold(q_ptr)) return;


    /* Make an object */
    o_idx = o_pop();

    /* Success */
    if (o_idx)
    {
        object_type *o_ptr;

        /* Acquire object */
        o_ptr = &o_list[o_idx];

        /* Copy the object */
        object_copy(o_ptr, q_ptr);

        /* Save location */
        o_ptr->iy = y;
        o_ptr->ix = x;

        /* Build a stack */
        o_ptr->next_o_idx = c_ptr->o_idx;

        /* Place the object */
        c_ptr->o_idx = o_idx;

        /* Notice */
        note_spot(y, x);

        /* Redraw */
        lite_spot(y, x);
    }
}


/*
 * Let an object fall to the ground at or near a location.
 *
 * The initial location is assumed to be "in_bounds()".
 *
 * This function takes a parameter "chance". This is the percentage
 * chance that the item will "disappear" instead of drop. If the object
 * has been thrown, then this is the chance of disappearance on contact.
 *
 * Hack -- this function uses "chance" to determine if it should produce
 * some form of "description" of the drop event (under the player).
 *
 * We check several locations to see if we can find a location at which
 * the object can combine, stack, or be placed. Artifacts will try very
 * hard to be placed, including "teleporting" to a useful grid if needed.
 */
s16b drop_near(object_type *j_ptr, int chance, int y, int x)
{
    int i, k, d, s;

    int bs, bn;
    int by, bx;
    int dy, dx;
    int ty, tx = 0;

    s16b o_idx = 0;

    s16b this_o_idx, next_o_idx = 0;

    cave_type *c_ptr;

    char o_name[MAX_NLEN];

    bool flag = FALSE;
    bool done = FALSE;

    /* Extract plural */
    bool plural = (j_ptr->number != 1);

    /* Describe object */
    object_desc(o_name, j_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));


    /* Handle normal "breakage" */
    if (!object_is_artifact(j_ptr) && (randint0(100) < chance))
    {
        /* Message */
        msg_format("The %s disappear%s.",
               o_name, (plural ? "" : "s"));


        /* Debug */
        if (p_ptr->wizard) msg_print("(breakage)");

        stats_on_m_destroy(j_ptr, 1);

        /* Failure */
        return (0);
    }


    /* Score */
    bs = -1;

    /* Picker */
    bn = 0;

    /* Default */
    by = y;
    bx = x;

    /* Scan local grids */
    for (dy = -3; dy <= 3; dy++)
    {
        /* Scan local grids */
        for (dx = -3; dx <= 3; dx++)
        {
            bool comb = FALSE;

            /* Calculate actual distance */
            d = (dy * dy) + (dx * dx);

            /* Ignore distant grids */
            if (d > 10) continue;

            /* Location */
            ty = y + dy;
            tx = x + dx;

            /* Skip illegal grids */
            if (!in_bounds(ty, tx)) continue;

            /* Require line of projection */
            if (!projectable(y, x, ty, tx)) continue;

            /* Obtain grid */
            c_ptr = &cave[ty][tx];

            /* Require floor space */
            if (!cave_drop_bold(ty, tx)) continue;

            /* No objects */
            k = 0;

            /* Scan objects in that grid */
            for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
            {
                object_type *o_ptr;

                /* Acquire object */
                o_ptr = &o_list[this_o_idx];

                /* Acquire next object */
                next_o_idx = o_ptr->next_o_idx;

                /* Check for possible combination */
                if (object_similar(o_ptr, j_ptr)) comb = TRUE;

                /* Count objects */
                k++;
            }

            /* Add new object */
            if (!comb) k++;

            /* Paranoia */
            if (k > 99) continue;

            /* Calculate score */
            s = 1000 - (d + k * 5);

            /* Skip bad values */
            if (s < bs) continue;

            /* New best value */
            if (s > bs) bn = 0;

            /* Apply the randomizer to equivalent values */
            if ((++bn >= 2) && !one_in_(bn)) continue;

            /* Keep score */
            bs = s;

            /* Track it */
            by = ty;
            bx = tx;

            /* Okay */
            flag = TRUE;
        }
    }


    /* Handle lack of space */
    if (!flag && !object_is_artifact(j_ptr))
    {
        /* Message */
        msg_format("The %s disappear%s.",
               o_name, (plural ? "" : "s"));


        /* Debug */
        if (p_ptr->wizard) msg_print("(no floor space)");


        /* Failure */
        return (0);
    }


    /* Find a grid */
    for (i = 0; !flag && (i < 1000); i++)
    {
        /* Bounce around */
        ty = rand_spread(by, 1);
        tx = rand_spread(bx, 1);

        /* Verify location */
        if (!in_bounds(ty, tx)) continue;

        /* Bounce to that location */
        by = ty;
        bx = tx;

        /* Require floor space */
        if (!cave_drop_bold(by, bx)) continue;

        /* Okay */
        flag = TRUE;
    }


    if (!flag)
    {
        int candidates = 0, pick;

        for (ty = 1; ty < cur_hgt - 1; ty++)
        {
            for (tx = 1; tx < cur_wid - 1; tx++)
            {
                /* A valid space found */
                if (cave_drop_bold(ty, tx)) candidates++;
            }
        }

        /* No valid place! */
        if (!candidates)
        {
            /* Message */
            msg_format("The %s disappear%s.", o_name, (plural ? "" : "s"));

            /* Debug */
            if (p_ptr->wizard) msg_print("(no floor space)");

            /* Mega-Hack -- preserve artifacts */
            if (preserve_mode)
            {
                /* Hack -- Preserve unknown artifacts */
                if (object_is_fixed_artifact(j_ptr) && !object_is_known(j_ptr))
                {
                    /* Mega-Hack -- Preserve the artifact */
                    a_info[j_ptr->name1].cur_num = 0;
                }

                if (j_ptr->name3 && !object_is_known(j_ptr))
                {
                    /* Mega-Hack -- Preserve the artifact */
                    a_info[j_ptr->name3].cur_num = 0;
                }
            }

            /* Failure */
            return 0;
        }

        /* Choose a random one */
        pick = randint1(candidates);

        for (ty = 1; ty < cur_hgt - 1; ty++)
        {
            for (tx = 1; tx < cur_wid - 1; tx++)
            {
                if (cave_drop_bold(ty, tx))
                {
                    pick--;

                    /* Is this a picked one? */
                    if (!pick) break;
                }
            }

            if (!pick) break;
        }

        by = ty;
        bx = tx;
    }


    /* Grid */
    c_ptr = &cave[by][bx];

    /* Scan objects in that grid for combination */
    for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
    {
        object_type *o_ptr;

        /* Acquire object */
        o_ptr = &o_list[this_o_idx];

        /* Acquire next object */
        next_o_idx = o_ptr->next_o_idx;

        /* Check for combination */
        if (object_similar(o_ptr, j_ptr))
        {
            /* Combine the items */
            object_absorb(o_ptr, j_ptr);

            /* Success */
            done = TRUE;

            /* Done */
            break;
        }
    }

    /* Get new object */
    if (!done) o_idx = o_pop();

    /* Failure */
    if (!done && !o_idx)
    {
        /* Message */
        msg_format("The %s disappear%s.",
               o_name, (plural ? "" : "s"));


        /* Debug */
        if (p_ptr->wizard) msg_print("(too many objects)");


        /* Hack -- Preserve artifacts */
        if (object_is_fixed_artifact(j_ptr))
        {
            a_info[j_ptr->name1].cur_num = 0;
        }

        if (j_ptr->name3)
        {
            a_info[j_ptr->name3].cur_num = 0;
        }

        /* Failure */
        return (0);
    }

    /* Stack */
    if (!done)
    {
        /* Structure copy */
        object_copy(&o_list[o_idx], j_ptr);

        /* Access new object */
        j_ptr = &o_list[o_idx];

        /* Locate */
        j_ptr->iy = by;
        j_ptr->ix = bx;

        /* No monster */
        j_ptr->held_m_idx = 0;

        /* Build a stack */
        j_ptr->next_o_idx = c_ptr->o_idx;

        /* Place the object */
        c_ptr->o_idx = o_idx;

        /* Success */
        done = TRUE;
    }

    /* Note the spot */
    note_spot(by, bx);

    /* Draw the spot */
    lite_spot(by, bx);

    /* Sound */
    sound(SOUND_DROP);

    /* Mega-Hack -- no message if "dropped" by player */
    /* Message when an object falls under the player */
    if (chance && player_bold(by, bx))
    {
        msg_print("You feel something roll beneath your feet.");

    }

    /* XXX XXX XXX */
    p_ptr->window |= PW_OBJECT_LIST;

    /* Result */
    return (o_idx);
}


/*
 * Scatter some "great" objects near the player
 */
void acquirement(int y1, int x1, int num, bool great, bool known)
{
    object_type *i_ptr;
    object_type object_type_body;
    u32b mode = AM_GOOD | (great ? (AM_GREAT | AM_TAILORED) : 0L);
    int  attempt = 0;

    /* Acquirement */
    while (num && attempt < 1000)
    {
        /* Get local object */
        i_ptr = &object_type_body;

        /* Wipe the object */
        object_wipe(i_ptr);
        attempt++;

        /* Make a good (or great) object (if possible) */
        if (!make_object(i_ptr, mode)) continue;

        num--;
        if (known)
        {
            object_aware(i_ptr);
            object_known(i_ptr);
        }

        /* Drop the object */
        (void)drop_near(i_ptr, -1, y1, x1);
    }
}


#define MAX_NORMAL_TRAPS 18

/* See init_feat_variables() in init2.c */
static s16b normal_traps[MAX_NORMAL_TRAPS];

/*
 * Initialize arrays for normal traps
 */
void init_normal_traps(void)
{
    int cur_trap = 0;

    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_TRAPDOOR");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_PIT");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_SPIKED_PIT");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_POISON_PIT");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_TY_CURSE");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_TELEPORT");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_FIRE");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_ACID");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_SLOW");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_LOSE_STR");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_LOSE_DEX");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_LOSE_CON");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_BLIND");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_CONFUSE");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_POISON");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_SLEEP");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_TRAPS");
    normal_traps[cur_trap++] = f_tag_to_index_in_init("TRAP_ALARM");
}

/*
 * Get random trap
 *
 * XXX XXX XXX This routine should be redone to reflect trap "level".
 * That is, it does not make sense to have spiked pits at 50 feet.
 * Actually, it is not this routine, but the "trap instantiation"
 * code, which should also check for "trap doors" on quest levels.
 */
s16b choose_random_trap(void)
{
    s16b feat;

    /* Pick a trap */
    while (1)
    {
        /* Hack -- pick a trap */
        feat = normal_traps[randint0(MAX_NORMAL_TRAPS)];

        /* Accept non-trapdoors */
        if (!have_flag(f_info[feat].flags, FF_MORE)) break;

        /* Hack -- no trap doors on special levels */
        if (p_ptr->inside_arena || quest_number(dun_level)) continue;

        /* Hack -- no trap doors on the deepest level */
        if (dun_level >= d_info[dungeon_type].maxdepth) continue;

        break;
    }

    return feat;
}

/*
 * Disclose an invisible trap
 */
void disclose_grid(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    if (cave_have_flag_grid(c_ptr, FF_SECRET))
    {
        /* No longer hidden */
        cave_alter_feat(y, x, FF_SECRET);
    }
    else if (c_ptr->mimic)
    {
        /* No longer hidden */
        c_ptr->mimic = 0;

        /* Notice */
        note_spot(y, x);

        /* Redraw */
        lite_spot(y, x);
    }
}


/*
 * Places a random trap at the given location.
 *
 * The location must be a legal, naked, floor grid.
 *
 * Note that all traps start out as "invisible" and "untyped", and then
 * when they are "discovered" (by detecting them or setting them off),
 * the trap is "instantiated" as a visible, "typed", trap.
 */
void place_trap(int y, int x)
{
    cave_type *c_ptr = &cave[y][x];

    /* Paranoia -- verify location */
    if (!in_bounds(y, x)) return;

    /* Require empty, clean, floor grid */
    if (!cave_clean_bold(y, x)) return;

    /* Place an invisible trap */
    c_ptr->mimic = c_ptr->feat;
    c_ptr->feat = choose_random_trap();
}


/*
 * Describe the charges on an item in the inventory.
 */
void inven_item_charges(int item)
{
    object_type *o_ptr = &inventory[item];
    int          charges;

    if (!object_is_device(o_ptr)) return;
    if (!object_is_known(o_ptr)) return;
    if (!o_ptr->activation.cost) return; /* Just checking ... */

    charges = device_sp(o_ptr) / o_ptr->activation.cost;

    if (charges == 1)
        msg_print("You have 1 charge remaining.");
    else
        msg_format("You have %d charges remaining.", charges);
}


/*
 * Describe an item in the inventory.
 */
void inven_item_describe(int item)
{
    object_type *o_ptr = &inventory[item];
    char        o_name[MAX_NLEN];

    object_desc(o_name, o_ptr, OD_COLOR_CODED);
    msg_format("You have %s.", o_name);
}


/*
 * Increase the "number" of an item in the inventory
 */
void inven_item_increase(int item, int num)
{
    object_type *o_ptr = &inventory[item];

    /* Apply */
    num += o_ptr->number;

    /* Bounds check */
    if (num > 255) num = 255;
    else if (num < 0) num = 0;

    /* Un-apply */
    num -= o_ptr->number;

    /* Change the number and weight */
    if (num)
    {
        /* Add the number */
        o_ptr->number += num;

        /* Add the weight */
        p_ptr->total_weight += (num * o_ptr->weight);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Recalculate mana XXX */
        p_ptr->update |= (PU_MANA);

        /* Combine the pack */
        p_ptr->notice |= (PN_COMBINE);

        /* Window stuff */
        p_ptr->window |= (PW_INVEN | PW_EQUIP);
    }
}


/*
 * Erase an inventory slot if it has no more items
 */
void inven_item_optimize(int item)
{
    object_type *o_ptr = &inventory[item];

    /* Only optimize real items */
    if (!o_ptr->k_idx) return;

    /* Only optimize empty items */
    if (o_ptr->number) return;

    /* The item is in the pack */
    if (item <= INVEN_PACK)
    {
        int i;

        /* One less item */
        inven_cnt--;

        /* Slide everything down */
        for (i = item; i < INVEN_PACK; i++)
        {
            /* Structure copy */
            inventory[i] = inventory[i+1];
        }

        /* Erase the "final" slot */
        object_wipe(&inventory[i]);

        /* Window stuff */
        p_ptr->window |= (PW_INVEN);
    }

    /* The item is being wielded */
    else
    {
        /* Erase the empty slot */
        object_wipe(&inventory[item]);

        /* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS);

        /* Recalculate torch */
        p_ptr->update |= (PU_TORCH);

        /* Recalculate mana XXX */
        p_ptr->update |= (PU_MANA);

        /* Window stuff */
        p_ptr->window |= (PW_EQUIP);
    }

    /* Window stuff */
    p_ptr->window |= (PW_SPELL);
}


/*
 * Describe the charges on an item on the floor.
 */
void floor_item_charges(int item)
{
    object_type *o_ptr = &o_list[item];
    int          charges;

    if (!object_is_device(o_ptr)) return;
    if (!object_is_known(o_ptr)) return;
    if (!o_ptr->activation.cost) return; /* Just checking ... */

    charges = device_sp(o_ptr) / o_ptr->activation.cost;

    if (charges == 1)
        msg_print("There is 1 charge remaining.");
    else
        msg_format("There are %d charges remaining.", charges);
}

/*
 * Describe an item in the inventory.
 */
void floor_item_describe(int item)
{
    object_type *o_ptr = &o_list[item];
    char        o_name[MAX_NLEN];

    object_desc(o_name, o_ptr, OD_COLOR_CODED);
    msg_format("You see %s.", o_name);
}


/*
 * Increase the "number" of an item on the floor
 */
void floor_item_increase(int item, int num)
{
    object_type *o_ptr = &o_list[item];

    /* Apply */
    num += o_ptr->number;

    /* Bounds check */
    if (num > 255) num = 255;
    else if (num < 0) num = 0;

    /* Un-apply */
    num -= o_ptr->number;

    /* Change the number */
    o_ptr->number += num;
}


/*
 * Optimize an item on the floor (destroy "empty" items)
 */
void floor_item_optimize(int item)
{
    object_type *o_ptr = &o_list[item];

    /* Paranoia -- be sure it exists */
    if (!o_ptr->k_idx) return;

    /* Only optimize empty items */
    if (o_ptr->number) return;

    /* Delete the object */
    delete_object_idx(item);
}


/*
 * Check if we have space for an item in the pack without overflow
 */
bool inven_carry_okay(object_type *o_ptr)
{
    int j;

    /* Empty slot? */
    if (inven_cnt < INVEN_PACK) return (TRUE);
    
    /* Similar slot? */
    for (j = 0; j < INVEN_PACK; j++)
    {
        object_type *j_ptr = &inventory[j];

        /* Skip non-objects */
        if (!j_ptr->k_idx) continue;

        /* Check if the two items can be combined */
        if (object_similar(j_ptr, o_ptr)) return (TRUE);
    }

    /* Nope */
    return (FALSE);
}


bool object_sort_comp(object_type *o_ptr, s32b o_value, object_type *j_ptr)
{
    int o_type, j_type;

    /* Use empty slots */
    if (!j_ptr->k_idx) return TRUE;

    /* Hack -- readable books always come first */
    if ((o_ptr->tval == REALM1_BOOK) &&
        (j_ptr->tval != REALM1_BOOK)) return TRUE;
    if ((j_ptr->tval == REALM1_BOOK) &&
        (o_ptr->tval != REALM1_BOOK)) return FALSE;

    if ((o_ptr->tval == REALM2_BOOK) &&
        (j_ptr->tval != REALM2_BOOK)) return TRUE;
    if ((j_ptr->tval == REALM2_BOOK) &&
        (o_ptr->tval != REALM2_BOOK)) return FALSE;

    /* Objects sort by decreasing type */
    if (o_ptr->tval > j_ptr->tval) return TRUE;
    if (o_ptr->tval < j_ptr->tval) return FALSE;

    /* Non-aware (flavored) items always come last */
    /* Can happen in the home */
    if (!object_is_aware(o_ptr)) return FALSE;
    if (!object_is_aware(j_ptr)) return TRUE;

    /* Objects sort by increasing sval */
    if (o_ptr->sval < j_ptr->sval) return TRUE;
    if (o_ptr->sval > j_ptr->sval) return FALSE;

    /* Unidentified objects always come last */
    /* Objects in the home can be unknown */
    if (!object_is_known(o_ptr)) return FALSE;
    if (!object_is_known(j_ptr)) return TRUE;

    /* Fixed artifacts, random artifacts and ego items */
    if (object_is_fixed_artifact(o_ptr)) o_type = 3;
    else if (o_ptr->art_name) o_type = 2;
    else if (object_is_ego(o_ptr)) o_type = 1;
    else o_type = 0;

    if (!object_is_device(o_ptr))
    {
        if (object_is_fixed_artifact(j_ptr)) j_type = 3;
        else if (j_ptr->art_name) j_type = 2;
        else if (object_is_ego(j_ptr)) j_type = 1;
        else j_type = 0;

        if (o_type < j_type) return TRUE;
        if (o_type > j_type) return FALSE;
    }

    switch (o_ptr->tval)
    {
    case TV_FIGURINE:
    case TV_STATUE:
    case TV_CORPSE:
    case TV_CAPTURE:
        if (r_info[o_ptr->pval].level < r_info[j_ptr->pval].level) return TRUE;
        if ((r_info[o_ptr->pval].level == r_info[j_ptr->pval].level) && (o_ptr->pval < j_ptr->pval)) return TRUE;
        return FALSE;

    case TV_SHOT:
    case TV_ARROW:
    case TV_BOLT:
        /* Objects sort by increasing hit/damage bonuses */
        if (o_ptr->to_h + o_ptr->to_d < j_ptr->to_h + j_ptr->to_d) return TRUE;
        if (o_ptr->to_h + o_ptr->to_d > j_ptr->to_h + j_ptr->to_d) return FALSE;
        break;

    case TV_ROD:
    case TV_WAND:
    case TV_STAFF:
        if (o_ptr->activation.type < j_ptr->activation.type) return TRUE;
        if (o_ptr->activation.type > j_ptr->activation.type) return FALSE;
        if (device_level(o_ptr) < device_level(j_ptr)) return TRUE;
        if (device_level(o_ptr) > device_level(j_ptr)) return FALSE;
        break;
    }

    /* Objects sort by decreasing value */
    return o_value > object_value(j_ptr);
}


/*
 * Add an item to the players inventory, and return the slot used.
 *
 * If the new item can combine with an existing item in the inventory,
 * it will do so, using "object_similar()" and "object_absorb()", else,
 * the item will be placed into the "proper" location in the inventory.
 *
 * This function can be used to "over-fill" the player's pack, but only
 * once, and such an action must trigger the "overflow" code immediately.
 * Note that when the pack is being "over-filled", the new item must be
 * placed into the "overflow" slot, and the "overflow" must take place
 * before the pack is reordered, but (optionally) after the pack is
 * combined. This may be tricky. See "dungeon.c" for info.
 *
 * Note that this code must remove any location/stack information
 * from the object once it is placed into the inventory.
 */
s16b inven_carry(object_type *o_ptr)
{
    int i, j, k;
    int n = -1;

    object_type *j_ptr;

    /* Check for combining */
    for (j = 0; j < INVEN_PACK; j++)
    {
        j_ptr = &inventory[j];

        /* Skip non-objects */
        if (!j_ptr->k_idx) continue;

        /* Hack -- track last item */
        n = j;

        /* Check if the two items can be combined */
        if (object_similar(j_ptr, o_ptr))
        {
            stats_on_pickup(o_ptr);

            /* Combine the items */
            object_absorb(j_ptr, o_ptr);

            /* Increase the weight */
            p_ptr->total_weight += (o_ptr->number * o_ptr->weight);

            /* Recalculate bonuses */
            p_ptr->update |= (PU_BONUS);

            /* Window stuff */
            p_ptr->window |= (PW_INVEN);

            /* Success */
            return (j);
        }
    }


    /* Paranoia */
    if (inven_cnt > INVEN_PACK) return (-1);

    /* Find an empty slot */
    for (j = 0; j <= INVEN_PACK; j++)
    {
        j_ptr = &inventory[j];

        /* Use it if found */
        if (!j_ptr->k_idx) break;
    }

    /* Use that slot */
    i = j;


    /* Reorder the pack */
    if (i < INVEN_PACK)
    {
        /* Get the "value" of the item */
        s32b o_value = object_value(o_ptr);

        /* Scan every occupied slot */
        for (j = 0; j < INVEN_PACK; j++)
        {
            if (object_sort_comp(o_ptr, o_value, &inventory[j])) break;
        }

        /* Use that slot */
        i = j;

        /* Slide objects */
        for (k = n; k >= i; k--)
        {
            /* Hack -- Slide the item */
            object_copy(&inventory[k+1], &inventory[k]);
        }

        /* Wipe the empty slot */
        object_wipe(&inventory[i]);
    }


    /* Copy the item */
    stats_on_pickup(o_ptr);
    object_copy(&inventory[i], o_ptr);

    /* Access new object */
    j_ptr = &inventory[i];

    /* Forget stack */
    j_ptr->next_o_idx = 0;

    /* Forget monster */
    j_ptr->held_m_idx = 0;

    /* Forget location */
    j_ptr->iy = j_ptr->ix = 0;

    /* Player touches it, and no longer marked */
    j_ptr->marked &= (OM_WORN | OM_COUNTED | OM_EFFECT_COUNTED | OM_EGO_COUNTED | OM_ART_COUNTED);  /* Ah, but remember the "worn" status ... */
    j_ptr->marked |= OM_TOUCHED;

    /* Increase the weight */
    p_ptr->total_weight += (j_ptr->number * j_ptr->weight);

    /* Count the items */
    inven_cnt++;

    /* Recalculate bonuses */
    p_ptr->update |= (PU_BONUS);

    /* Combine and Reorder pack */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Window stuff */
    p_ptr->window |= (PW_INVEN);

    /* Return the slot */
    return (i);
}


/*
 * Take off (some of) a non-cursed equipment item
 *
 * Note that only one item at a time can be wielded per slot.
 *
 * Note that taking off an item when "full" may cause that item
 * to fall to the ground.
 *
 * Return the inventory slot into which the item is placed.
 */
s16b inven_takeoff(int item, int amt)
{
    int slot;

    object_type forge;
    object_type *q_ptr;

    object_type *o_ptr;

    cptr act = "";

    char o_name[MAX_NLEN];


    /* Get the item to take off */
    o_ptr = &inventory[item];

    /* Paranoia */
    if (amt <= 0) return (-1);

    /* Verify */
    if (amt > o_ptr->number) amt = o_ptr->number;

    /* Get local object */
    q_ptr = &forge;

    /* Obtain a local object */
    object_copy(q_ptr, o_ptr);

    /* Modify quantity */
    q_ptr->number = amt;

    /* Describe the object */
    object_desc(o_name, q_ptr, 0);

    if (equip_is_valid_slot(item))
        act = "You were wearing";

    /* Modify, Optimize */
    inven_item_increase(item, -amt);
    inven_item_optimize(item);

    /* Carry the object */
    slot = inven_carry(q_ptr);

    /* Message */
    msg_format("%s %s (%c).", act, o_name, index_to_label(slot));


    /* Return slot */
    return (slot);
}


/*
 * Drop (some of) a non-cursed inventory/equipment item
 *
 * The object will be dropped "near" the current location
 */
void inven_drop(int item, int amt)
{
    object_type  forge;
    object_type *q_ptr;
    object_type *o_ptr;
    int          old_number;
    char         o_name[MAX_NLEN];


    /* Access original object */
    o_ptr = &inventory[item];

    /* Error check */
    if (amt <= 0) return;

    /* Not too many */
    if (amt > o_ptr->number) amt = o_ptr->number;


    /* Take off equipment */
    if (equip_is_valid_slot(item))
    {
        /* Take off first */
        item = inven_takeoff(item, amt);

        /* Access original object */
        o_ptr = &inventory[item];
    }


    /* Get local object */
    q_ptr = &forge;

    /* Obtain local object */
    object_copy(q_ptr, o_ptr);

    /* Distribute charges of wands or rods */
    distribute_charges(o_ptr, q_ptr, amt);

    /* Modify quantity */
    old_number = q_ptr->number;
    q_ptr->number = amt;
    q_ptr->marked &= ~OM_WORN;

    /* Describe local object */
    object_desc(o_name, q_ptr, OD_COLOR_CODED);

    /* Message */
    msg_format("You drop %s (%c).", o_name, index_to_label(item));

    /* Drop it near the player */
    (void)drop_near(q_ptr, 0, py, px);

    /* Modify, Describe, Optimize */
    inven_item_increase(item, -amt);
    if (amt < old_number)
        inven_item_describe(item);
    inven_item_optimize(item);

    /* Runes confer benefits even when in inventory */
    p_ptr->update |= PU_BONUS;
}


/*
 * Combine items in the pack
 *
 * Note special handling of the "overflow" slot
 */
void combine_pack(void)
{
    int             i, j, k;
    object_type     *o_ptr;
    object_type     *j_ptr;
    bool            flag = FALSE, combined;

    do
    {
        combined = FALSE;

        /* Combine the pack (backwards) */
        for (i = INVEN_PACK; i > 0; i--)
        {
            /* Get the item */
            o_ptr = &inventory[i];

            /* Skip empty items */
            if (!o_ptr->k_idx) continue;

            /* Scan the items above that item */
            for (j = 0; j < i; j++)
            {
                int max_num;

                /* Get the item */
                j_ptr = &inventory[j];

                /* Skip empty items */
                if (!j_ptr->k_idx) continue;

                /*
                 * Get maximum number of the stack if these
                 * are similar, get zero otherwise.
                 */
                max_num = object_similar_part(j_ptr, o_ptr);

                /* Can we (partialy) drop "o_ptr" onto "j_ptr"? */
                if (max_num && j_ptr->number < max_num)
                {
                    stats_on_combine(j_ptr, o_ptr);
                    if (o_ptr->number + j_ptr->number <= max_num)
                    {
                        /* Take note */
                        flag = TRUE;

                        /* Add together the item counts */
                        object_absorb(j_ptr, o_ptr);

                        /* One object is gone */
                        inven_cnt--;

                        /* Slide everything down */
                        for (k = i; k < INVEN_PACK; k++)
                        {
                            /* Structure copy */
                            inventory[k] = inventory[k+1];
                        }

                        /* Erase the "final" slot */
                        object_wipe(&inventory[k]);
                    }
                    else
                    {
                        int old_num = o_ptr->number;
                        int remain = j_ptr->number + o_ptr->number - max_num;
#if 0
                        o_ptr->number -= remain;
#endif
                        /* Add together the item counts */
                        object_absorb(j_ptr, o_ptr);

                        o_ptr->number = remain;

                        /* Hack -- if rods are stacking, add the pvals (maximum timeouts) and current timeouts together. -LM- */
                        if (o_ptr->tval == TV_ROD)
                        {
                            o_ptr->pval =  o_ptr->pval * remain / old_num;
                            o_ptr->timeout = o_ptr->timeout * remain / old_num;
                        }

                        /* Hack -- if wands are stacking, combine the charges. -LM- */
                        if (o_ptr->tval == TV_WAND)
                        {
                            o_ptr->pval = o_ptr->pval * remain / old_num;
                        }
                    }

                    /* Window stuff */
                    p_ptr->window |= (PW_INVEN);

                    /* Take note */
                    combined = TRUE;

                    /* Done */
                    break;
                }
            }
        }
    }
    while (combined);

    /* Message */
    if (flag) msg_print("You combine some items in your pack.");
}


/*
 * Reorder items in the pack
 *
 * Note special handling of the "overflow" slot
 */
void reorder_pack(void)
{
    int             i, j, k;
    s32b            o_value;
    object_type     forge;
    object_type     *q_ptr;
    object_type     *o_ptr;
    bool            flag = FALSE;


    /* Re-order the pack (forwards) */
    for (i = 0; i < INVEN_PACK; i++)
    {
        /* Mega-Hack -- allow "proper" over-flow */
        if ((i == INVEN_PACK) && (inven_cnt == INVEN_PACK)) break;

        /* Get the item */
        o_ptr = &inventory[i];

        /* Skip empty slots */
        if (!o_ptr->k_idx) continue;

        /* Get the "value" of the item */
        o_value = object_value(o_ptr);

        /* Scan every occupied slot */
        for (j = 0; j < INVEN_PACK; j++)
        {
            if (object_sort_comp(o_ptr, o_value, &inventory[j])) break;
        }

        /* Never move down */
        if (j >= i) continue;

        /* Take note */
        flag = TRUE;

        /* Get local object */
        q_ptr = &forge;

        /* Save a copy of the moving item */
        object_copy(q_ptr, &inventory[i]);

        /* Slide the objects */
        for (k = i; k > j; k--)
        {
            /* Slide the item */
            object_copy(&inventory[k], &inventory[k-1]);
        }

        /* Insert the moving item */
        object_copy(&inventory[j], q_ptr);

        /* Window stuff */
        p_ptr->window |= (PW_INVEN);
    }

    /* Message */
    if (flag) msg_print("You reorder some items in your pack.");

}


/*
 * Hack -- display an object kind in the current window
 *
 * Include list of usable spells for readible books
 */
void display_koff(int k_idx)
{
    int y;

    object_type forge;
    object_type *q_ptr;
    int         sval;
    int         use_realm;
    rect_t      display = {0};

    char o_name[MAX_NLEN];

    display.x = 0;
    display.y = 2;
    display.cy = Term->hgt - 2;
    display.cx = Term->wid;


    /* Erase the window */
    for (y = 0; y < Term->hgt; y++)
    {
        /* Erase the line */
        Term_erase(0, y, 255);
    }

    /* No info */
    if (!k_idx) return;

    /* Get local object */
    q_ptr = &forge;

    /* Prepare the object */
    object_prep(q_ptr, k_idx);

    /* Describe */
    object_desc(o_name, q_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY | OD_STORE));

    /* Mention the object name */
    Term_putstr(0, 0, -1, TERM_WHITE, o_name);

    /* Access the item's sval */
    sval = q_ptr->sval;
    use_realm = tval2realm(q_ptr->tval);

    /* Warriors are illiterate */
    if (p_ptr->realm1 || p_ptr->realm2)
    {
        if ((use_realm != p_ptr->realm1) && (use_realm != p_ptr->realm2)) return;
    }
    else
    {
        if ((p_ptr->pclass != CLASS_SORCERER) && (p_ptr->pclass != CLASS_RED_MAGE)) return;
        if (!is_magic(use_realm)) return;
        if ((p_ptr->pclass == CLASS_RED_MAGE) && (use_realm != REALM_ARCANE) && (sval > 1)) return;
    }

    /* Display spells in readible books */
    {
        int     spell = -1;
        int     num = 0;
        byte    spells[64];

        /* Extract spells */
        for (spell = 0; spell < 32; spell++)
        {
            /* Check for this spell */
            if (fake_spell_flags[sval] & (1L << spell))
            {
                /* Collect this spell */
                spells[num++] = spell;
            }
        }

        /* Print spells */
        print_spells(0, spells, num, display, use_realm);
    }
}

/* Choose one of items that have warning flag */
static bool _object_has_warning(object_type *o_ptr) {
    u32b flgs[TR_FLAG_SIZE];
    object_flags(o_ptr, flgs);
    return have_flag(flgs, TR_WARNING);
}
object_type *choose_warning_item(void)
{
    int slot = equip_random_slot(_object_has_warning);
    if (slot)
        return equip_obj(slot);
    return NULL;
}

/* Calculate spell damages */
static void spell_damcalc(monster_type *m_ptr, int typ, int dam, int limit, int *max)
{
    monster_race *r_ptr = &r_info[m_ptr->r_idx];
    int          rlev = r_ptr->level;
    bool         ignore_wraith_form = FALSE;

    if (limit) dam = (dam > limit) ? limit : dam;

    /* Vulnerability, resistance and immunity */
    switch (typ)
    {
    case GF_ELEC:
        dam = res_calc_dam(RES_ELEC, dam);
        if (dam == 0) 
            ignore_wraith_form = TRUE;
        break;

    case GF_POIS:
        dam = res_calc_dam(RES_POIS, dam);
        break;

    case GF_ACID:
        dam = res_calc_dam(RES_ACID, dam);
        if (dam == 0) 
            ignore_wraith_form = TRUE;
        break;

    case GF_COLD:
    case GF_ICE:
        dam = res_calc_dam(RES_COLD, dam);
        if (dam == 0) 
            ignore_wraith_form = TRUE;
        break;

    case GF_FIRE:
        dam = res_calc_dam(RES_FIRE, dam);
        if (dam == 0) 
            ignore_wraith_form = TRUE;
        break;

    case GF_PSY_SPEAR:
        ignore_wraith_form = TRUE;
        break;

    case GF_ARROW:
        if (!p_ptr->blind && equip_find_artifact(ART_ZANTETSU))
        {
            dam = 0;
            ignore_wraith_form = TRUE;
        }
        break;

    case GF_LITE:
        dam = res_calc_dam(RES_LITE, dam);
        /*
         * Cannot use "ignore_wraith_form" strictly (for "random one damage")
         * "dam *= 2;" for later "dam /= 2"
         */
        if (IS_WRAITH()) dam *= 2;
        break;

    case GF_DARK:
        dam = res_calc_dam(RES_DARK, dam);
        break;

    case GF_SHARDS:
        dam = res_calc_dam(RES_SHARDS, dam);
        break;

    case GF_SOUND:
        dam = res_calc_dam(RES_SOUND, dam);
        break;

    case GF_CONFUSION:
        dam = res_calc_dam(RES_CONF, dam);
        break;

    case GF_CHAOS:
        dam = res_calc_dam(RES_CHAOS, dam);
        break;

    case GF_NETHER:
        dam = res_calc_dam(RES_NETHER, dam);
        break;

    case GF_DISENCHANT:
        dam = res_calc_dam(RES_DISEN, dam);
        break;

    case GF_NEXUS:
        dam = res_calc_dam(RES_NEXUS, dam);
        break;

    case GF_TIME:
        dam = res_calc_dam(RES_TIME, dam);
        break;

    case GF_GRAVITY:
        if (p_ptr->levitation) dam = (dam * 2) / 3;
        break;

    case GF_ROCKET:
        dam = res_calc_dam(RES_SHARDS, dam);
        break;

    case GF_NUKE:
        dam = res_calc_dam(RES_POIS, dam);
        break;

    case GF_DEATH_RAY:
        if (get_race()->flags & RACE_IS_NONLIVING)
        {
            dam = 0;
            ignore_wraith_form = TRUE;
        }
        break;

    case GF_HOLY_FIRE:
        if (p_ptr->align > 10) dam /= 2;
        else if (p_ptr->align < -10) dam *= 2;
        break;

    case GF_HELL_FIRE:
        if (p_ptr->align > 10) dam *= 2;
        break;

    case GF_MIND_BLAST:
    case GF_BRAIN_SMASH:
        if (100 + rlev / 2 <= MAX(5, p_ptr->skills.sav))
        {
            dam = 0;
            ignore_wraith_form = TRUE;
        }
        break;

    case GF_CAUSE_1:
    case GF_CAUSE_2:
    case GF_CAUSE_3:
    case GF_HAND_DOOM:
        if (100 + rlev / 2 <= p_ptr->skills.sav)
        {
            dam = 0;
            ignore_wraith_form = TRUE;
        }
        break;

    case GF_CAUSE_4:
        if ((100 + rlev / 2 <= p_ptr->skills.sav) && (m_ptr->r_idx != MON_KENSHIROU))
        {
            dam = 0;
            ignore_wraith_form = TRUE;
        }
        break;
    }

    if (IS_WRAITH() && !ignore_wraith_form)
    {
        dam /= 2;
        if (!dam) dam = 1;
    }

    if (dam > *max) *max = dam;
}

/* Calculate blow damages */
static int blow_damcalc(monster_type *m_ptr, monster_blow *blow_ptr)
{
    int  dam = blow_ptr->d_dice * blow_ptr->d_side;
    int  dummy_max = 0;
    bool check_wraith_form = TRUE;

    if (blow_ptr->method != RBM_EXPLODE)
    {
        int ac = p_ptr->ac + p_ptr->to_a;

        switch (blow_ptr->effect)
        {
        case RBE_SUPERHURT:
        {
            int tmp_dam = dam - (dam * ((ac < 150) ? ac : 150) / 250);
            dam = MAX(dam, tmp_dam * 2);
            break;
        }

        case RBE_HURT:
        case RBE_SHATTER:
            dam -= (dam * ((ac < 150) ? ac : 150) / 250);
            break;

        case RBE_ACID:
            spell_damcalc(m_ptr, GF_ACID, dam, 0, &dummy_max);
            dam = dummy_max;
            check_wraith_form = FALSE;
            break;

        case RBE_ELEC:
            spell_damcalc(m_ptr, GF_ELEC, dam, 0, &dummy_max);
            dam = dummy_max;
            check_wraith_form = FALSE;
            break;

        case RBE_FIRE:
            spell_damcalc(m_ptr, GF_FIRE, dam, 0, &dummy_max);
            dam = dummy_max;
            check_wraith_form = FALSE;
            break;

        case RBE_COLD:
            spell_damcalc(m_ptr, GF_COLD, dam, 0, &dummy_max);
            dam = dummy_max;
            check_wraith_form = FALSE;
            break;

        case RBE_DR_MANA:
            dam = 0;
            check_wraith_form = FALSE;
            break;
        }

        if (check_wraith_form && IS_WRAITH())
        {
            dam /= 2;
            if (!dam) dam = 1;
        }
    }
    else
    {
        dam = (dam + 1) / 2;
        spell_damcalc(m_ptr, mbe_info[blow_ptr->effect].explode_type, dam, 0, &dummy_max);
        dam = dummy_max;
    }

    return dam;
}

/* Examine the grid (xx,yy) and warn the player if there are any danger */
bool process_warning(int xx, int yy)
{
    int mx, my;
    cave_type *c_ptr;
    char o_name[MAX_NLEN];

#define WARNING_AWARE_RANGE 12
    int dam_max = 0;
    static int old_damage = 0;

    for (mx = xx - WARNING_AWARE_RANGE; mx < xx + WARNING_AWARE_RANGE + 1; mx++)
    {
        for (my = yy - WARNING_AWARE_RANGE; my < yy + WARNING_AWARE_RANGE + 1; my++)
        {
            int dam_max0 = 0;
            monster_type *m_ptr;
            monster_race *r_ptr;

            if (!in_bounds(my, mx) || (distance(my, mx, yy, xx) > WARNING_AWARE_RANGE)) continue;

            c_ptr = &cave[my][mx];

            if (!c_ptr->m_idx) continue;

            m_ptr = &m_list[c_ptr->m_idx];

            if (MON_CSLEEP(m_ptr)) continue;
            if (!is_hostile(m_ptr)) continue;
            if (!is_aware(m_ptr)) continue;

            r_ptr = &r_info[m_ptr->r_idx];

            /* Monster spells (only powerful ones)*/
            if (projectable(my, mx, yy, xx))
            {
                int breath_dam_div3 = m_ptr->hp / 3;
                int breath_dam_div6 = m_ptr->hp / 6;
                u32b f4 = r_ptr->flags4;
                u32b f5 = r_ptr->flags5;
                u32b f6 = r_ptr->flags6;

                if (!(d_info[dungeon_type].flags1 & DF1_NO_MAGIC))
                {
                    int rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);
                    int storm_dam = rlev * 4 + 150;
                    bool powerful = (bool)(r_ptr->flags2 & RF2_POWERFUL);

                    if (f4 & RF4_BA_CHAO) spell_damcalc(m_ptr, GF_CHAOS, rlev * (powerful ? 3 : 2) + 100, 0, &dam_max0);
                    if (f5 & RF5_BA_MANA) spell_damcalc(m_ptr, GF_MANA, storm_dam, 0, &dam_max0);
                    if (f5 & RF5_BA_DARK) spell_damcalc(m_ptr, GF_DARK, storm_dam, 0, &dam_max0);
                    if (f5 & RF5_BA_LITE) spell_damcalc(m_ptr, GF_LITE, storm_dam, 0, &dam_max0);
                    if (f6 & RF6_HAND_DOOM) spell_damcalc(m_ptr, GF_HAND_DOOM, p_ptr->chp * 6 / 10, 0, &dam_max0);
                    if (f6 & RF6_PSY_SPEAR) spell_damcalc(m_ptr, GF_PSY_SPEAR, powerful ? (rlev * 2 + 150) : (rlev * 3 / 2 + 100), 0, &dam_max0);
                }
                if (f4 & RF4_ROCKET) spell_damcalc(m_ptr, GF_ROCKET, m_ptr->hp / 4, 800, &dam_max0);
                if (f4 & RF4_BR_ACID) spell_damcalc(m_ptr, GF_ACID, breath_dam_div3, 1600, &dam_max0);
                if (f4 & RF4_BR_ELEC) spell_damcalc(m_ptr, GF_ELEC, breath_dam_div3, 1600, &dam_max0);
                if (f4 & RF4_BR_FIRE) spell_damcalc(m_ptr, GF_FIRE, breath_dam_div3, 1600, &dam_max0);
                if (f4 & RF4_BR_COLD) spell_damcalc(m_ptr, GF_COLD, breath_dam_div3, 1600, &dam_max0);
                if (f4 & RF4_BR_POIS) spell_damcalc(m_ptr, GF_POIS, breath_dam_div3, 800, &dam_max0);
                if (f4 & RF4_BR_NETH) spell_damcalc(m_ptr, GF_NETHER, breath_dam_div6, 550, &dam_max0);
                if (f4 & RF4_BR_LITE) spell_damcalc(m_ptr, GF_LITE, breath_dam_div6, 400, &dam_max0);
                if (f4 & RF4_BR_DARK) spell_damcalc(m_ptr, GF_DARK, breath_dam_div6, 400, &dam_max0);
                if (f4 & RF4_BR_CONF) spell_damcalc(m_ptr, GF_CONFUSION, breath_dam_div6, 450, &dam_max0);
                if (f4 & RF4_BR_SOUN) spell_damcalc(m_ptr, GF_SOUND, breath_dam_div6, 450, &dam_max0);
                if (f4 & RF4_BR_CHAO) spell_damcalc(m_ptr, GF_CHAOS, breath_dam_div6, 600, &dam_max0);
                if (f4 & RF4_BR_DISE) spell_damcalc(m_ptr, GF_DISENCHANT, breath_dam_div6, 500, &dam_max0);
                if (f4 & RF4_BR_NEXU) spell_damcalc(m_ptr, GF_NEXUS, breath_dam_div3, 250, &dam_max0);
                if (f4 & RF4_BR_TIME) spell_damcalc(m_ptr, GF_TIME, breath_dam_div3, 150, &dam_max0);
                if (f4 & RF4_BR_INER) spell_damcalc(m_ptr, GF_INERT, breath_dam_div6, 200, &dam_max0);
                if (f4 & RF4_BR_GRAV) spell_damcalc(m_ptr, GF_GRAVITY, breath_dam_div3, 200, &dam_max0);
                if (f4 & RF4_BR_SHAR) spell_damcalc(m_ptr, GF_SHARDS, breath_dam_div6, 500, &dam_max0);
                if (f4 & RF4_BR_PLAS) spell_damcalc(m_ptr, GF_PLASMA, breath_dam_div6, 150, &dam_max0);
                if (f4 & RF4_BR_WALL) spell_damcalc(m_ptr, GF_FORCE, breath_dam_div6, 200, &dam_max0);
                if (f4 & RF4_BR_MANA) spell_damcalc(m_ptr, GF_MANA, breath_dam_div3, 250, &dam_max0);
                if (f4 & RF4_BR_NUKE) spell_damcalc(m_ptr, GF_NUKE, breath_dam_div3, 800, &dam_max0);
                if (f4 & RF4_BR_DISI) spell_damcalc(m_ptr, GF_DISINTEGRATE, breath_dam_div6, 150, &dam_max0);
            }

            /* Monster melee attacks */
            if (!(r_ptr->flags1 & RF1_NEVER_BLOW) && !(d_info[dungeon_type].flags1 & DF1_NO_MELEE))
            {
                if (mx <= xx + 1 && mx >= xx - 1 && my <= yy + 1 && my >= yy - 1)
                {
                    int m;
                    int dam_melee = 0;
                    for (m = 0; m < 4; m++)
                    {
                        /* Skip non-attacks */
                        if (!r_ptr->blow[m].method || (r_ptr->blow[m].method == RBM_SHOOT)) continue;

                        /* Extract the attack info */
                        dam_melee += blow_damcalc(m_ptr, &r_ptr->blow[m]);
                        if (r_ptr->blow[m].method == RBM_EXPLODE) break;
                    }
                    if (dam_melee > dam_max0) dam_max0 = dam_melee;
                }
            }

            /* Contribution from this monster */
            dam_max += dam_max0;
        }
    }

    /* Prevent excessive warning */
    if (dam_max > old_damage)
    {
        old_damage = dam_max * 3 / 2;

        if (dam_max > p_ptr->chp / 2)
        {
            object_type *o_ptr = choose_warning_item();

            if (o_ptr) object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
            else strcpy(o_name, "body"); /* Warning ability without item */
            msg_format("Your %s pulsates sharply!", o_name);
            disturb(0, 0);
            return get_check("Really want to go ahead? ");
        }
    }
    else old_damage = old_damage / 2;

    c_ptr = &cave[yy][xx];
    if (((!easy_disarm && is_trap(c_ptr->feat))
        || (c_ptr->mimic && is_trap(c_ptr->feat))) && !one_in_(13))
    {
        object_type *o_ptr = choose_warning_item();

        if (o_ptr) object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
        else strcpy(o_name, "body"); /* Warning ability without item */
        msg_format("Your %s pulsates!", o_name);
        disturb(0, 0);
        return get_check("Really want to go ahead? ");
    }

    return TRUE;
}


static bool item_tester_hook_melee_ammo(object_type *o_ptr)
{
    switch (o_ptr->tval)
    {
        case TV_HAFTED:
        case TV_POLEARM:
        case TV_DIGGING:
        case TV_BOLT:
        case TV_ARROW:
        case TV_SHOT:
        {
            return (TRUE);
        }
        case TV_SWORD:
        {
            if (o_ptr->sval != SV_DOKUBARI) return (TRUE);
        }
    }

    return (FALSE);
}


/*
 *  A structure for smithing
 */
typedef struct {
    int add;       /* TR flag number or special essence id */
    cptr add_name; /* Name of this ability */
    int type;      /* Menu number */
    int essence;   /* Index for carrying essences */
    int value;     /* Needed value to add this ability */
} essence_type;


/*
 *  Smithing type data for Weapon smith
 */
static essence_type essence_info[] = 
{
    {TR_STR, "strength", 4, TR_STR, 20},
    {TR_INT, "intelligence", 4, TR_INT, 20},
    {TR_WIS, "wisdom", 4, TR_WIS, 20},
    {TR_DEX, "dexterity", 4, TR_DEX, 20},
    {TR_CON, "constitution", 4, TR_CON, 20},
    {TR_CHR, "charisma", 4, TR_CHR, 20},
    {TR_MAGIC_MASTERY, "magic mastery", 4, TR_MAGIC_MASTERY, 20},
    {TR_STEALTH, "stealth", 4, TR_STEALTH, 40},
    {TR_SEARCH, "serching", 4, TR_SEARCH, 15},
    {TR_INFRA, "infravision", 4, TR_INFRA, 15},
    {TR_TUNNEL, "digging", 4, TR_TUNNEL, 15},
    {TR_SPEED, "speed", 4, TR_SPEED, 12},
    {TR_BLOWS, "extra attack", 1, TR_BLOWS, 20},
    {TR_CHAOTIC, "chaos brand", 1, TR_CHAOTIC, 15},
    {TR_VAMPIRIC, "vampiric brand", 1, TR_VAMPIRIC, 60},
    {TR_IMPACT, "quake activation", 7, TR_IMPACT, 15},
    {TR_BRAND_POIS, "poison brand", 1, TR_BRAND_POIS, 20},
    {TR_BRAND_ACID, "acid brand", 1, TR_BRAND_ACID, 20},
    {TR_BRAND_ELEC, "electric brand", 1, TR_BRAND_ELEC, 20},
    {TR_BRAND_FIRE, "fire brand", 1, TR_BRAND_FIRE, 20},
    {TR_BRAND_COLD, "cold brand", 1, TR_BRAND_COLD, 20},
    {TR_SUST_STR, "sustain strength", 3, TR_SUST_STR, 15},
    {TR_SUST_INT, "sustain intelligence", 3, TR_SUST_STR, 15},
    {TR_SUST_WIS, "sustain wisdom", 3, TR_SUST_STR, 15},
    {TR_SUST_DEX, "sustain dexterity", 3, TR_SUST_STR, 15},
    {TR_SUST_CON, "sustain constitution", 3, TR_SUST_STR, 15},
    {TR_SUST_CHR, "sustain charisma", 3, TR_SUST_STR, 15},
    {TR_IM_ACID, "acid immunity", 2, TR_IM_ACID, 20},
    {TR_IM_ELEC, "electric immunity", 2, TR_IM_ACID, 20},
    {TR_IM_FIRE, "fire immunity", 2, TR_IM_ACID, 20},
    {TR_IM_COLD, "cold immunity", 2, TR_IM_ACID, 20},
    {TR_REFLECT, "reflection", 2, TR_REFLECT, 20},
    {TR_FREE_ACT, "free action", 3, TR_FREE_ACT, 20},
    {TR_HOLD_LIFE, "hold life", 3, TR_HOLD_LIFE, 20},
    {TR_RES_ACID, "resistance to acid", 2, TR_RES_ACID, 15},
    {TR_RES_ELEC, "resistance to electric", 2, TR_RES_ELEC, 15},
    {TR_RES_FIRE, "resistance to fire", 2, TR_RES_FIRE, 15},
    {TR_RES_COLD, "resistance to cold", 2, TR_RES_COLD, 15},
    {TR_RES_POIS, "resistance to poison", 2, TR_RES_POIS, 25},
    {TR_RES_FEAR, "resistance to fear", 2, TR_RES_FEAR, 20},
    {TR_RES_LITE, "resistance to light", 2, TR_RES_LITE, 20},
    {TR_RES_DARK, "resistance to dark", 2, TR_RES_DARK, 20},
    {TR_RES_BLIND, "resistance to blind", 2, TR_RES_BLIND, 20},
    {TR_RES_CONF, "resistance to confusion", 2, TR_RES_CONF, 20},
    {TR_RES_SOUND, "resistance to sound", 2, TR_RES_SOUND, 20},
    {TR_RES_SHARDS, "resistance to shard", 2, TR_RES_SHARDS, 20},
    {TR_RES_NETHER, "resistance to nether", 2, TR_RES_NETHER, 20},
    {TR_RES_NEXUS, "resistance to nexus", 2, TR_RES_NEXUS, 20},
    {TR_RES_CHAOS, "resistance to chaos", 2, TR_RES_CHAOS, 20},
    {TR_RES_DISEN, "resistance to disenchantment", 2, TR_RES_DISEN, 20},
    {TR_SH_FIRE, "", 0, -2, 0},
    {TR_SH_ELEC, "", 0, -2, 0},
    {TR_SH_COLD, "", 0, -2, 0},
    {TR_NO_MAGIC, "anti magic", 3, TR_NO_MAGIC, 15},
    {TR_WARNING, "warning", 3, TR_WARNING, 20},
    {TR_LEVITATION, "levitation", 3, TR_LEVITATION, 20},
    {TR_LITE, "permanent light", 3, TR_LITE, 15},
    {TR_SEE_INVIS, "see invisible", 3, TR_SEE_INVIS, 20},
    {TR_TELEPATHY, "telepathy", 6, TR_TELEPATHY, 15},
    {TR_SLOW_DIGEST, "slow digestion", 3, TR_SLOW_DIGEST, 15},
    {TR_REGEN, "regeneration", 3, TR_REGEN, 20},
    {TR_TELEPORT, "teleport", 3, TR_TELEPORT, 25},

    {TR_SLAY_EVIL, "slay evil", 5, TR_SLAY_EVIL, 100},
    {TR_SLAY_ANIMAL, "slay animal", 5, TR_SLAY_ANIMAL, 20},
    {TR_KILL_ANIMAL, "kill animal", 5, TR_SLAY_ANIMAL, 60},
    {TR_KILL_EVIL, "kill evil", 0, TR_SLAY_EVIL, 60},
    {TR_SLAY_UNDEAD, "slay undead", 5, TR_SLAY_UNDEAD, 20},
    {TR_KILL_UNDEAD, "kill undead", 5, TR_SLAY_UNDEAD, 60},
    {TR_SLAY_DEMON, "slay demon", 5, TR_SLAY_DEMON, 20},
    {TR_KILL_DEMON, "kill demon", 5, TR_SLAY_DEMON, 60},
    {TR_SLAY_ORC, "slay orc", 5, TR_SLAY_ORC, 15},
    {TR_KILL_ORC, "kill orc", 5, TR_SLAY_ORC, 60},
    {TR_SLAY_TROLL, "slay troll", 5, TR_SLAY_TROLL, 15},
    {TR_KILL_TROLL, "kill troll", 5, TR_SLAY_TROLL, 60},
    {TR_SLAY_GIANT, "slay giant", 5, TR_SLAY_GIANT, 20},
    {TR_KILL_GIANT, "kill giant", 5, TR_SLAY_GIANT, 60},       
    {TR_SLAY_DRAGON, "slay dragon", 5, TR_SLAY_DRAGON, 20},
    {TR_KILL_DRAGON, "kill dragon", 5, TR_SLAY_DRAGON, 60},
    {TR_SLAY_HUMAN, "slay human", 5, TR_SLAY_HUMAN, 20},
    {TR_KILL_HUMAN, "kill human", 5, TR_SLAY_HUMAN, 60},

    {TR_ESP_ANIMAL, "sense animal", 6, TR_SLAY_ANIMAL, 40},
    {TR_ESP_UNDEAD, "sense undead", 6, TR_SLAY_UNDEAD, 40}, 
    {TR_ESP_DEMON, "sense demon", 6, TR_SLAY_DEMON, 40},       
    {TR_ESP_ORC, "sense orc", 6, TR_SLAY_ORC, 40},     
    {TR_ESP_TROLL, "sense troll", 6, TR_SLAY_TROLL, 40},   
    {TR_ESP_GIANT, "sense giant", 6, TR_SLAY_GIANT, 40},       
    {TR_ESP_DRAGON, "sense dragon", 6, TR_SLAY_DRAGON, 40},
    {TR_ESP_HUMAN, "sense human", 6, TR_SLAY_HUMAN, 40},

    {ESSENCE_ATTACK, "weapon enchant", 10, TR_ES_ATTACK, 30},
    {ESSENCE_AC, "armor enchant", 10, TR_ES_AC, 15},
    {ESSENCE_TMP_RES_ACID, "resist acid activation", 7, TR_RES_ACID, 50},
    {ESSENCE_TMP_RES_ELEC, "resist electricity activation", 7, TR_RES_ELEC, 50},
    {ESSENCE_TMP_RES_FIRE, "resist fire activation", 7, TR_RES_FIRE, 50},
    {ESSENCE_TMP_RES_COLD, "resist cold activation", 7, TR_RES_COLD, 50},
    {ESSENCE_SH_FIRE, "fiery sheath", 7, -1, 50},
    {ESSENCE_SH_ELEC, "electric sheath", 7, -1, 50},
    {ESSENCE_SH_COLD, "sheath of coldness", 7, -1, 50},
    {ESSENCE_RESISTANCE, "resistance", 2, -1, 150},
    {ESSENCE_SUSTAIN, "elements proof", 10, -1, 10},
    {ESSENCE_SLAY_GLOVE, "gauntlets of slaying", 1, TR_ES_ATTACK, 200},

    {-1, NULL, 0, -1, 0}
};


/*
 *  Essense names for Weapon smith
 */

static cptr essence_name[] = 
{
    "strength",
    "intelligen.",
    "wisdom",
    "dexterity",
    "constitut.",
    "charisma",
    "magic mast.",
    "",
    "stealth",
    "serching",
    "infravision",
    "digging",
    "speed",
    "extra atk",
    "chaos brand",
    "vampiric",
    "slay animal",
    "slay evil",
    "slay undead",
    "slay demon",
    "slay orc",
    "slay troll",
    "slay giant",
    "slay dragon",
    "",
    "",
    "quake",
    "pois. brand",
    "acid brand",
    "elec. brand",
    "fire brand",
    "cold brand",
    "sustain",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "immunity",
    "",
    "",
    "",
    "",
    "reflection",
    "free action",
    "hold life",
    "res. acid",
    "res. elec.",
    "res. fire",
    "res. cold",
    "res. poison",
    "res. fear",
    "res. light",
    "res. dark",
    "res. blind",
    "res.confuse",
    "res. sound",
    "res. shard",
    "res. nether",
    "res. nexus",
    "res. chaos",
    "res. disen.",
    "",
    "",
    "slay human",
    "",
    "",
    "anti magic",
    "",
    "",
    "warning",
    "",
    "",
    "",
    "levitation",
    "perm. light",
    "see invis.",
    "telepathy",
    "slow dige.",
    "regen.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "teleport",
    "",
    "",
    "weapon enc.",
    "armor enc.",

    NULL
};


static void display_essence(void)
{
    int i, num = 0;

    screen_save();
    for (i = 1; i < 22; i++)
    {
        prt("",i,0);
    }
    prt("Essence      Num      Essence      Num      Essence      Num ", 1, 8);
    for (i = 0; essence_name[i]; i++)
    {
        if (!essence_name[i][0]) continue;
        prt(format("%-11s %5d", essence_name[i], p_ptr->magic_num1[i]), 2+num%21, 8+num/21*22);
        num++;
    }
    prt("List of all essences you have.", 0, 0);
    (void)inkey();
    screen_load();
    return;
}

static void drain_essence(void)
{
    int drain_value[MAX_MAGIC_NUM];
    int i, item;
    int dec = 4;
    bool observe = FALSE;
    int old_ds, old_dd, old_to_h, old_to_d, old_ac, old_to_a, old_pval, old_name2, old_timeout;
    u32b old_flgs[TR_FLAG_SIZE], new_flgs[TR_FLAG_SIZE];
    object_type *o_ptr;
    cptr            q, s;
    byte iy, ix, marked, number;
    s16b next_o_idx, weight;

    for (i = 0; i < MAX_MAGIC_NUM; i++)
        drain_value[i] = 0;

    item_tester_hook = object_is_weapon_armour_ammo;
    item_tester_no_ryoute = TRUE;

    /* Get an item */
    q = "Extract from which item? ";
    s = "You have nothing you can extract from.";

    if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    if (object_is_known(o_ptr) && !object_is_nameless(o_ptr))
    {
        char o_name[MAX_NLEN];
        object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
        if (!get_check(format("Really extract from %s? ", o_name))) return;
    }

    energy_use = 100;

    object_flags(o_ptr, old_flgs);
    if (have_flag(old_flgs, TR_KILL_DRAGON)) add_flag(old_flgs, TR_SLAY_DRAGON);
    if (have_flag(old_flgs, TR_KILL_ANIMAL)) add_flag(old_flgs, TR_SLAY_ANIMAL);
    if (have_flag(old_flgs, TR_KILL_EVIL)) add_flag(old_flgs, TR_SLAY_EVIL);
    if (have_flag(old_flgs, TR_KILL_UNDEAD)) add_flag(old_flgs, TR_SLAY_UNDEAD);
    if (have_flag(old_flgs, TR_KILL_DEMON)) add_flag(old_flgs, TR_SLAY_DEMON);
    if (have_flag(old_flgs, TR_KILL_ORC)) add_flag(old_flgs, TR_SLAY_ORC);
    if (have_flag(old_flgs, TR_KILL_TROLL)) add_flag(old_flgs, TR_SLAY_TROLL);
    if (have_flag(old_flgs, TR_KILL_GIANT)) add_flag(old_flgs, TR_SLAY_GIANT);
    if (have_flag(old_flgs, TR_KILL_HUMAN)) add_flag(old_flgs, TR_SLAY_HUMAN);

    old_to_a = o_ptr->to_a;
    old_ac = o_ptr->ac;
    old_to_h = o_ptr->to_h;
    old_to_d = o_ptr->to_d;
    old_ds = o_ptr->ds;
    old_dd = o_ptr->dd;
    old_pval = o_ptr->pval;
    old_name2 = o_ptr->name2;
    old_timeout = o_ptr->timeout;
    if (o_ptr->curse_flags & (TRC_CURSED | TRC_HEAVY_CURSE | TRC_PERMA_CURSE)) dec--;
    if (have_flag(old_flgs, TR_AGGRAVATE)) dec--;
    if (have_flag(old_flgs, TR_NO_TELE)) dec--;
    if (have_flag(old_flgs, TR_DRAIN_EXP)) dec--;
    if (have_flag(old_flgs, TR_TY_CURSE)) dec--;

    iy = o_ptr->iy;
    ix = o_ptr->ix;
    next_o_idx = o_ptr->next_o_idx;
    marked = o_ptr->marked;
    weight = o_ptr->weight;
    number = o_ptr->number;

    object_prep(o_ptr, o_ptr->k_idx);

    o_ptr->iy=iy;
    o_ptr->ix=ix;
    o_ptr->next_o_idx=next_o_idx;
    o_ptr->marked=marked;
    o_ptr->number = number;
    if (o_ptr->tval == TV_DRAG_ARMOR) o_ptr->timeout = old_timeout;
    if (item >= 0) p_ptr->total_weight += (o_ptr->weight*o_ptr->number - weight*number);
    o_ptr->ident |= (IDENT_FULL);
    object_aware(o_ptr);
    object_known(o_ptr);

    object_flags(o_ptr, new_flgs);

    for (i = 0; essence_info[i].add_name; i++)
    {
        essence_type *es_ptr = &essence_info[i];
        int pval = 0;

        if (es_ptr->add < TR_FLAG_MAX && is_pval_flag(es_ptr->add) && old_pval)
            pval = (have_flag(new_flgs, es_ptr->add)) ? old_pval - o_ptr->pval : old_pval;

        if (es_ptr->add < TR_FLAG_MAX &&
            (!have_flag(new_flgs, es_ptr->add) || pval) &&
            have_flag(old_flgs, es_ptr->add))
        {
            if (pval)
            {
                drain_value[es_ptr->essence] += 10 * pval;
            }
            else if (es_ptr->essence != -2)
            {
                drain_value[es_ptr->essence] += 10;
            }
            else if (es_ptr->add == TR_SH_FIRE)
            {
                drain_value[TR_BRAND_FIRE] += 10;
                drain_value[TR_RES_FIRE] += 10;
            }
            else if (es_ptr->add == TR_SH_ELEC)
            {
                drain_value[TR_BRAND_ELEC] += 10;
                drain_value[TR_RES_ELEC] += 10;
            }
            else if (es_ptr->add == TR_SH_COLD)
            {
                drain_value[TR_BRAND_COLD] += 10;
                drain_value[TR_RES_COLD] += 10;
            }
        }
    }

    if ((have_flag(old_flgs, TR_FORCE_WEAPON)) && !(have_flag(new_flgs, TR_FORCE_WEAPON)))
    {
        drain_value[TR_INT] += 5;
        drain_value[TR_WIS] += 5;
    }
    if ((have_flag(old_flgs, TR_VORPAL)) && !(have_flag(new_flgs, TR_VORPAL)))
    {
        drain_value[TR_BRAND_POIS] += 5;
        drain_value[TR_BRAND_ACID] += 5;
        drain_value[TR_BRAND_ELEC] += 5;
        drain_value[TR_BRAND_FIRE] += 5;
        drain_value[TR_BRAND_COLD] += 5;
    }
    if ((have_flag(old_flgs, TR_DEC_MANA)) && !(have_flag(new_flgs, TR_DEC_MANA)))
    {
        drain_value[TR_INT] += 10;
    }
    if ((have_flag(old_flgs, TR_XTRA_MIGHT)) && !(have_flag(new_flgs, TR_XTRA_MIGHT)))
    {
        drain_value[TR_STR] += 10;
    }
    if ((have_flag(old_flgs, TR_XTRA_SHOTS)) && !(have_flag(new_flgs, TR_XTRA_SHOTS)))
    {
        drain_value[TR_DEX] += 10;
    }
    if (old_name2 == EGO_GLOVES_GENJI)
    {
        drain_value[TR_DEX] += 20;
    }
    if (object_is_weapon_ammo(o_ptr))
    {
        if (old_ds > o_ptr->ds) drain_value[TR_ES_ATTACK] += (old_ds-o_ptr->ds)*10;

        if (old_dd > o_ptr->dd) drain_value[TR_ES_ATTACK] += (old_dd-o_ptr->dd)*10;
    }
    if (old_to_h > o_ptr->to_h) drain_value[TR_ES_ATTACK] += (old_to_h-o_ptr->to_h)*10;
    if (old_to_d > o_ptr->to_d) drain_value[TR_ES_ATTACK] += (old_to_d-o_ptr->to_d)*10;
    if (old_ac > o_ptr->ac) drain_value[TR_ES_AC] += (old_ac-o_ptr->ac)*10;
    if (old_to_a > o_ptr->to_a) drain_value[TR_ES_AC] += (old_to_a-o_ptr->to_a)*10;

    for (i = 0; i < MAX_MAGIC_NUM; i++)
    {
        drain_value[i] *= number;
        drain_value[i] = drain_value[i] * dec / 4;
        drain_value[i] = MAX(drain_value[i], 0);
        if ((o_ptr->tval >= TV_SHOT) && (o_ptr->tval <= TV_BOLT)) drain_value[i] /= 10;
        if (drain_value[i])
        {
            observe = TRUE;
        }
    }
    if (!observe)
    {
        msg_print("You were not able to extract any essence.");
    }
    else
    {
        msg_print("Extracted essences:");
        for (i = 0; essence_name[i]; i++)
        {
            if (!essence_name[i][0]) continue;
            if (!drain_value[i]) continue;

            p_ptr->magic_num1[i] += drain_value[i];
            p_ptr->magic_num1[i] = MIN(20000, p_ptr->magic_num1[i]);
            msg_print(NULL);
            msg_format("%s...%d", essence_name[i], drain_value[i]);
        }
    }

    /* Combine the pack */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Window stuff */
    p_ptr->window |= (PW_INVEN);
}



static int choose_essence(void)
{
    int mode = 0;
    char choice;
    int menu_line = (use_menu ? 1 : 0);

    cptr menu_name[] = {
        "Brand weapon",
        "Resistance",
        "Ability",
        "Magic number", 
        "Slay",
        "ESP",
        "Others"
    };
    const int mode_max = 7;

#ifdef ALLOW_REPEAT
    if (repeat_pull(&mode) && 1 <= mode && mode <= mode_max)
        return mode;
    mode = 0;
#endif /* ALLOW_REPEAT */

    if (use_menu)
    {
        screen_save();

        while(!mode)
        {
            int i;
            for (i = 0; i < mode_max; i++)
                prt(format(" %s %s", (menu_line == 1+i) ? "> " : "  ", menu_name[i]), 2 + i, 14);
            prt("Choose from menu.", 0, 0);

            choice = inkey();
            switch(choice)
            {
            case ESCAPE:
            case 'z':
            case 'Z':
                screen_load();
                return 0;
            case '2':
            case 'j':
            case 'J':
                menu_line++;
                break;
            case '8':
            case 'k':
            case 'K':
                menu_line += mode_max - 1;
                break;
            case '\r':
            case '\n':
            case 'x':
            case 'X':
                mode = menu_line;
                break;
            }
            if (menu_line > mode_max) menu_line -= mode_max;
        }
        screen_load();
    }
    else
    {
        screen_save();
        while (!mode)
        {
            int i;

            for (i = 0; i < mode_max; i++)
                prt(format("  %c) %s", 'a' + i, menu_name[i]), 2 + i, 14);

            if (!get_com("Command :", &choice, TRUE))
            {
                screen_load();
                return 0;
            }

            if (isupper(choice)) choice = tolower(choice);

            if ('a' <= choice && choice <= 'a' + (char)mode_max - 1)
                mode = (int)choice - 'a' + 1;
        }
        screen_load();
    }

#ifdef ALLOW_REPEAT
    repeat_push(mode);
#endif /* ALLOW_REPEAT */
    return mode;
}

static void add_essence(int mode)
{
    int item, max_num = 0;
    int i;
    bool flag,redraw;
    char choice;
    cptr            q, s;
    object_type *o_ptr;
    int ask = TRUE;
    char out_val[160];
    int num[22];
    char o_name[MAX_NLEN];
    int use_essence;
    essence_type *es_ptr;

    int menu_line = (use_menu ? 1 : 0);

    for (i = 0; essence_info[i].add_name; i++)
    {
        es_ptr = &essence_info[i];

        if (es_ptr->type != mode) continue;
        num[max_num++] = i;
    }

#ifdef ALLOW_REPEAT
    if (!repeat_pull(&i) || i<0 || i>=max_num)
    {
#endif /* ALLOW_REPEAT */


    /* Nothing chosen yet */
    flag = FALSE;

    /* No redraw yet */
    redraw = FALSE;

    /* Build a prompt */
    (void)strnfmt(out_val, 78, "(*=List, ESC=exit) Add which ability? ");
    if (use_menu) screen_save();

    /* Get a spell from the user */

    choice = (always_show_list || use_menu) ? ESCAPE:1;
    while (!flag)
    {
        bool able[22];
        if( choice==ESCAPE ) choice = ' '; 
        else if( !get_com(out_val, &choice, FALSE) )break; 

        if (use_menu && choice != ' ')
        {
            switch(choice)
            {
                case '0':
                {
                    screen_load();
                    return;
                }

                case '8':
                case 'k':
                case 'K':
                {
                    menu_line += (max_num-1);
                    break;
                }

                case '2':
                case 'j':
                case 'J':
                {
                    menu_line++;
                    break;
                }

                case '4':
                case 'h':
                case 'H':
                {
                    menu_line = 1;
                    break;
                }
                case '6':
                case 'l':
                case 'L':
                {
                    menu_line = max_num;
                    break;
                }

                case 'x':
                case 'X':
                case '\r':
                case '\n':
                {
                    i = menu_line - 1;
                    ask = FALSE;
                    break;
                }
            }
            if (menu_line > max_num) menu_line -= max_num;
        }
        /* Request redraw */
        if ((choice == ' ') || (choice == '*') || (choice == '?') || (use_menu && ask))
        {
            /* Show the list */
            if (!redraw || use_menu)
            {
                byte y, x = 10;
                int ctr;
                char dummy[80], dummy2[80];
                byte col;

                strcpy(dummy, "");

                /* Show list */
                redraw = TRUE;

                /* Save the screen */
                if (!use_menu) screen_save();

                for (y = 1; y < 24; y++)
                    prt("", y, x);

                /* Print header(s) */
                prt(format("   %-43s %6s/%s", "Ability (needed essence)", "Needs", "Possess"), 1, x);
                /* Print list */
                for (ctr = 0; ctr < max_num; ctr++)
                {
                    es_ptr = &essence_info[num[ctr]];

                    if (use_menu)
                    {
                        if (ctr == (menu_line-1))
                            strcpy(dummy, ">  ");
                        else strcpy(dummy, "   ");
                        
                    }
                    /* letter/number for power selection */
                    else
                    {
                        sprintf(dummy, "%c) ",I2A(ctr));
                    }

                    strcat(dummy, es_ptr->add_name);

                    col = TERM_WHITE;
                    able[ctr] = TRUE;

                    if (es_ptr->essence != -1)
                    {
                        strcat(dummy, format("(%s)", essence_name[es_ptr->essence]));
                        if (p_ptr->magic_num1[es_ptr->essence] < es_ptr->value) able[ctr] = FALSE;
                    }
                    else
                    {
                        switch(es_ptr->add)
                        {
                        case ESSENCE_SH_FIRE:
                            strcat(dummy, "(brand fire + res.fire)");
                            if (p_ptr->magic_num1[TR_BRAND_FIRE] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_FIRE] < es_ptr->value) able[ctr] = FALSE;
                            break;
                        case ESSENCE_SH_ELEC:
                            strcat(dummy, "(brand elec. + res. elec.)");
                            if (p_ptr->magic_num1[TR_BRAND_ELEC] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_ELEC] < es_ptr->value) able[ctr] = FALSE;
                            break;
                        case ESSENCE_SH_COLD:
                            strcat(dummy, "(brand cold + res. cold)");
                            if (p_ptr->magic_num1[TR_BRAND_COLD] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_COLD] < es_ptr->value) able[ctr] = FALSE;
                            break;
                        case ESSENCE_RESISTANCE:
                            strcat(dummy, "(r.fire+r.cold+r.elec+r.acid)");
                            if (p_ptr->magic_num1[TR_RES_FIRE] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_COLD] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_ELEC] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_ACID] < es_ptr->value) able[ctr] = FALSE;
                            break;
                        case ESSENCE_SUSTAIN:
                            strcat(dummy, "(r.fire+r.cold+r.elec+r.acid)");
                            if (p_ptr->magic_num1[TR_RES_FIRE] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_COLD] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_ELEC] < es_ptr->value) able[ctr] = FALSE;
                            if (p_ptr->magic_num1[TR_RES_ACID] < es_ptr->value) able[ctr] = FALSE;
                            break;
                        }
                    }

                    if (!able[ctr]) col = TERM_RED;

                    if (es_ptr->essence != -1)
                    {
                        sprintf(dummy2, "%-49s %3d/%d", dummy, es_ptr->value, (int)p_ptr->magic_num1[es_ptr->essence]);
                    }
                    else
                    {
                        sprintf(dummy2, "%-49s %3d/(\?\?)", dummy, es_ptr->value);
                    }

                    c_prt(col, dummy2, ctr+2, x);
                }
            }

            /* Hide the list */
            else
            {
                /* Hide list */
                redraw = FALSE;

                /* Restore the screen */
                screen_load();
            }

            /* Redo asking */
            continue;
        }

        if (!use_menu)
        {
            /* Note verify */
            ask = (isupper(choice));

            /* Lowercase */
            if (ask) choice = tolower(choice);

            /* Extract request */
            i = (islower(choice) ? A2I(choice) : -1);
        }

        /* Totally Illegal */
        if ((i < 0) || (i >= max_num) || !able[i])
        {
            bell();
            continue;
        }

        /* Verify it */
        if (ask)
        {
            char tmp_val[160];

            /* Prompt */
            (void) strnfmt(tmp_val, 78, "Add the abilitiy of %s? ", essence_info[num[i]].add_name);

            /* Belay that order */
            if (!get_check(tmp_val)) continue;
        }

        /* Stop the loop */
        flag = TRUE;
    }

    /* Restore the screen */
    if (redraw) screen_load();

    if (!flag) return;

#ifdef ALLOW_REPEAT
    repeat_push(i);
    }
#endif /* ALLOW_REPEAT */

    es_ptr = &essence_info[num[i]];

    if (es_ptr->add == ESSENCE_SLAY_GLOVE)
        item_tester_tval = TV_GLOVES;
    else if (mode == 1 || mode == 5)
        item_tester_hook = item_tester_hook_melee_ammo;
    else if (es_ptr->add == ESSENCE_ATTACK)
        item_tester_hook = object_allow_enchant_weapon;
    else if (es_ptr->add == ESSENCE_AC)
        item_tester_hook = object_is_armour;
    else
        item_tester_hook = object_is_weapon_armour_ammo;
    item_tester_no_ryoute = TRUE;

    /* Get an item */
    q = "Improve which item? ";
    s = "You have nothing to improve.";

    if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    if ((mode != 10) && (object_is_artifact(o_ptr) || object_is_smith(o_ptr)))
    {
        msg_print("This item is no more able to be improved.");
        return;
    }

    object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));

    use_essence = es_ptr->value;
    if ((o_ptr->tval >= TV_SHOT) && (o_ptr->tval <= TV_BOLT)) use_essence = (use_essence+9)/10;
    if (o_ptr->number > 1)
    {
        use_essence *= o_ptr->number;
        msg_format("It will take %d essences.",use_essence);

    }

    if (es_ptr->essence != -1)
    {
        if (p_ptr->magic_num1[es_ptr->essence] < use_essence)
        {
            msg_print("You don't have enough essences.");
            return;
        }
        if (is_pval_flag(es_ptr->add))
        {
            if (o_ptr->pval < 0)
            {
                msg_print("You cannot increase magic number of this item.");
                return;
            }
            else if (es_ptr->add == TR_BLOWS)
            {
                if (o_ptr->pval > 1)
                {
                    if (!get_check("The magic number of this weapon will become 1. Are you sure? ")) return;
                }

                o_ptr->pval = 1;
                msg_format("It will take %d essences.", use_essence);
            }
            else if (o_ptr->pval > 0)
            {
                use_essence *= o_ptr->pval;
                msg_format("It will take %d essences.", use_essence);
            }
            else
            {
                char tmp[80];
                char tmp_val[160];
                int pval;
                int limit = MIN(5, p_ptr->magic_num1[es_ptr->essence]/es_ptr->value);

                sprintf(tmp, "Enchant how many? (1-%d): ", limit);
                strcpy(tmp_val, "1");

                if (!get_string(tmp, tmp_val, 1)) return;
                pval = atoi(tmp_val);
                if (pval > limit) pval = limit;
                else if (pval < 1) pval = 1;
                o_ptr->pval += pval;
                use_essence *= pval;
                msg_format("It will take %d essences.", use_essence);
            }

            if (p_ptr->magic_num1[es_ptr->essence] < use_essence)
            {
                msg_print("You don't have enough essences.");
                return;
            }
        }
        else if (es_ptr->add == ESSENCE_SLAY_GLOVE)
        {
            char tmp_val[160];
            int val;
            int get_to_h, get_to_d;

            strcpy(tmp_val, "1");
            if (!get_string(format("Enchant how many? (1-%d):", p_ptr->lev/7+3), tmp_val, 2)) return;
            val = atoi(tmp_val);
            if (val > p_ptr->lev/7+3) val = p_ptr->lev/7+3;
            else if (val < 1) val = 1;
            use_essence *= val;
            msg_format("It will take %d essences.", use_essence);
            if (p_ptr->magic_num1[es_ptr->essence] < use_essence)
            {
                msg_print("You don't have enough essences.");
                return;
            }
            get_to_h = ((val+1)/2+randint0(val/2+1));
            get_to_d = ((val+1)/2+randint0(val/2+1));
            o_ptr->xtra4 = (get_to_h<<8)+get_to_d;
            o_ptr->to_h += get_to_h;
            o_ptr->to_d += get_to_d;
        }
        p_ptr->magic_num1[es_ptr->essence] -= use_essence;
        if (es_ptr->add == ESSENCE_ATTACK)
        {
            int max = 5 + 15*p_ptr->lev/50;

            /* Old: Everything could go to +15, +15
               New: Allow enchanting up to +20, +20, but this makes
                    shooting a bit strong. So clip ammo at +10, +10 */
            switch (o_ptr->tval)
            {
            case TV_SHOT:
            case TV_ARROW:
            case TV_BOLT:
                max = 10*p_ptr->lev/50;
                break;
            }

            if ( (o_ptr->tval == TV_BOW && o_ptr->sval == SV_HARP)
              || (o_ptr->tval == TV_SWORD && o_ptr->sval == SV_RUNESWORD) )
            {
                msg_print("You failed to enchant.");
                energy_use = 100;
                return;
            }

            if ((o_ptr->to_h >= max) && (o_ptr->to_d >= max))
            {
                msg_print("You failed to enchant.");
                energy_use = 100;
                return;
            }
            else
            {
                if (o_ptr->to_h < max) o_ptr->to_h++;
                if (o_ptr->to_d < max) o_ptr->to_d++;
            }
        }
        else if (es_ptr->add == ESSENCE_AC)
        {
            if ( o_ptr->name1 == ART_KAMIKAZE_ROBE
              || o_ptr->name2 == EGO_GLOVES_BERSERKER )
            {
                msg_print("You failed to enchant.");
                energy_use = 100;
                return;
            }

            if (o_ptr->to_a >= 5 + 15*p_ptr->lev/50)
            {
                msg_print("You failed to enchant.");
                energy_use = 100;
                return;
            }
            else
            {
                if (o_ptr->to_a < 5 + 15*p_ptr->lev/50) o_ptr->to_a++;
            }
        }
        else
        {
            o_ptr->xtra3 = es_ptr->add + 1;
        }
    }
    else
    {
        bool success = TRUE;

        switch(es_ptr->add)
        {
        case ESSENCE_SH_FIRE:
            if ((p_ptr->magic_num1[TR_BRAND_FIRE] < use_essence) || (p_ptr->magic_num1[TR_RES_FIRE] < use_essence))
            {
                success = FALSE;
                break;
            }
            p_ptr->magic_num1[TR_BRAND_FIRE] -= use_essence;
            p_ptr->magic_num1[TR_RES_FIRE] -= use_essence;
            break;
        case ESSENCE_SH_ELEC:
            if ((p_ptr->magic_num1[TR_BRAND_ELEC] < use_essence) || (p_ptr->magic_num1[TR_RES_ELEC] < use_essence))
            {
                success = FALSE;
                break;
            }
            p_ptr->magic_num1[TR_BRAND_ELEC] -= use_essence;
            p_ptr->magic_num1[TR_RES_ELEC] -= use_essence;
            break;
        case ESSENCE_SH_COLD:
            if ((p_ptr->magic_num1[TR_BRAND_COLD] < use_essence) || (p_ptr->magic_num1[TR_RES_COLD] < use_essence))
            {
                success = FALSE;
                break;
            }
            p_ptr->magic_num1[TR_BRAND_COLD] -= use_essence;
            p_ptr->magic_num1[TR_RES_COLD] -= use_essence;
            break;
        case ESSENCE_RESISTANCE:
        case ESSENCE_SUSTAIN:
            if ((p_ptr->magic_num1[TR_RES_ACID] < use_essence) || (p_ptr->magic_num1[TR_RES_ELEC] < use_essence) || (p_ptr->magic_num1[TR_RES_FIRE] < use_essence) || (p_ptr->magic_num1[TR_RES_COLD] < use_essence))
            {
                success = FALSE;
                break;
            }
            p_ptr->magic_num1[TR_RES_ACID] -= use_essence;
            p_ptr->magic_num1[TR_RES_ELEC] -= use_essence;
            p_ptr->magic_num1[TR_RES_FIRE] -= use_essence;
            p_ptr->magic_num1[TR_RES_COLD] -= use_essence;
            break;
        }
        if (!success)
        {
            msg_print("You don't have enough essences.");
            return;
        }
        if (es_ptr->add == ESSENCE_SUSTAIN)
        {
            add_flag(o_ptr->art_flags, TR_IGNORE_ACID);
            add_flag(o_ptr->art_flags, TR_IGNORE_ELEC);
            add_flag(o_ptr->art_flags, TR_IGNORE_FIRE);
            add_flag(o_ptr->art_flags, TR_IGNORE_COLD);
        }
        else
        {
            o_ptr->xtra3 = es_ptr->add + 1;
        }
    }

    energy_use = 100;

    msg_format("You have added ability of %s to %s.", es_ptr->add_name, o_name);

    /* Combine the pack */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Window stuff */
    p_ptr->window |= (PW_INVEN);
}


static void erase_essence(void)
{
    int item;
    cptr q, s;
    object_type *o_ptr;
    char o_name[MAX_NLEN];
    u32b flgs[TR_FLAG_SIZE];

    item_tester_hook = object_is_smith;

    /* Get an item */
    q = "Remove from which item? ";
    s = "You have nothing to remove essence.";

    if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

    /* Get the item (in the pack) */
    if (item >= 0)
    {
        o_ptr = &inventory[item];
    }

    /* Get the item (on the floor) */
    else
    {
        o_ptr = &o_list[0 - item];
    }

    object_desc(o_name, o_ptr, (OD_OMIT_PREFIX | OD_NAME_ONLY));
    if (!get_check(format("Are you sure? [%s]", o_name))) return;

    energy_use = 100;

    if (o_ptr->xtra3 == 1+ESSENCE_SLAY_GLOVE)
    {
        o_ptr->to_h -= (o_ptr->xtra4>>8);
        o_ptr->to_d -= (o_ptr->xtra4 & 0x000f);
        o_ptr->xtra4 = 0;
        if (o_ptr->to_h < 0 && o_ptr->name2 != EGO_GLOVES_BERSERKER) o_ptr->to_h = 0;
        if (o_ptr->to_d < 0) o_ptr->to_d = 0;
    }
    o_ptr->xtra3 = 0;
    object_flags(o_ptr, flgs);
    if (!(have_pval_flags(flgs))) o_ptr->pval = 0;
    msg_print("You removed all essence you have added.");

    /* Combine the pack */
    p_ptr->notice |= (PN_COMBINE | PN_REORDER);

    /* Window stuff */
    p_ptr->window |= (PW_INVEN);
}

void do_cmd_kaji(bool only_browse)
{
    int mode = 0;
    char choice;

    int menu_line = (use_menu ? 1 : 0);

    if (!only_browse)
    {
        if (p_ptr->confused)
        {
            msg_print("You are too confused!");

            return;
        }
        if (p_ptr->blind)
        {
            msg_print("You are blind!");

            return;
        }
        if (p_ptr->image)
        {
            msg_print("You are hallucinating!");

            return;
        }
    }

#ifdef ALLOW_REPEAT
    if (!(repeat_pull(&mode) && 1 <= mode && mode <= 5))
    {
#endif /* ALLOW_REPEAT */

    if (only_browse) screen_save();
    do {
    if (!only_browse) screen_save();
    if (use_menu)
    {
        while(!mode)
        {
            prt(format(" %s List essences", (menu_line == 1) ? "> " : "  "), 2, 14);
            prt(format(" %s Extract essence", (menu_line == 2) ? "> " : "  "), 3, 14);
            prt(format(" %s Remove essence", (menu_line == 3) ? "> " : "  "), 4, 14);
            prt(format(" %s Add essence", (menu_line == 4) ? "> " : "  "), 5, 14);
            prt(format(" %s Enchant weapon/armor", (menu_line == 5) ? "> " : "  "), 6, 14);
            prt(format("Choose command from menu."), 0, 0);
            choice = inkey();
            switch(choice)
            {
            case ESCAPE:
            case 'z':
            case 'Z':
                screen_load();
                return;
            case '2':
            case 'j':
            case 'J':
                menu_line++;
                break;
            case '8':
            case 'k':
            case 'K':
                menu_line+= 4;
                break;
            case '\r':
            case '\n':
            case 'x':
            case 'X':
                mode = menu_line;
                break;
            }
            if (menu_line > 5) menu_line -= 5;
        }
    }

    else
    {
        while (!mode)
        {
            prt("  a) List essences", 2, 14);
            prt("  b) Extract essence", 3, 14);
            prt("  c) Remove essence", 4, 14);
            prt("  d) Add essence", 5, 14);
            prt("  e) Enchant weapon/armor", 6, 14);
            if (!get_com("Command :", &choice, TRUE))
            {
                screen_load();
                return;
            }
            switch (choice)
            {
            case 'A':
            case 'a':
                mode = 1;
                break;
            case 'B':
            case 'b':
                mode = 2;
                break;
            case 'C':
            case 'c':
                mode = 3;
                break;
            case 'D':
            case 'd':
                mode = 4;
                break;
            case 'E':
            case 'e':
                mode = 5;
                break;
            }
        }
    }

    if (only_browse)
    {
        char temp[62*5];
        int line, j;

        /* Clear lines, position cursor  (really should use strlen here) */
        Term_erase(14, 21, 255);
        Term_erase(14, 20, 255);
        Term_erase(14, 19, 255);
        Term_erase(14, 18, 255);
        Term_erase(14, 17, 255);
        Term_erase(14, 16, 255);

        roff_to_buf(kaji_tips[mode-1], 62, temp, sizeof(temp));
        for(j=0, line = 17;temp[j];j+=(1+strlen(&temp[j])))
        {
            prt(&temp[j], line, 15);
            line++;
        }
        mode = 0;
    }
    if (!only_browse) screen_load();
    } while (only_browse);
#ifdef ALLOW_REPEAT
    repeat_push(mode);
    }
#endif /* ALLOW_REPEAT */

    switch(mode)
    {
        case 1: display_essence();break;
        case 2: drain_essence();break;
        case 3: erase_essence();break;
        case 4:
            mode = choose_essence();
            if (mode == 0)
                break;
            add_essence(mode);
            break;
        case 5: add_essence(10);break;
    }
}
