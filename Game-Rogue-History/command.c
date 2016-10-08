//Read and execute the user commands
//command.c   1.44    (A.I. Design)   2/14/85

#include "rogue.h"

static int lastcount;
static byte lastch, do_take, lasttake;

command()
{
  int ntimes;

  if (on(player, ISHASTE)) ntimes = rnd(2)+2;
  else ntimes = 1;
  while (ntimes--)
  {
    status();
    SIG2();
#ifdef WIZARD
    if (wizard) noscore = TRUE;
#endif
    if (no_command)
    {
      if (--no_command<=0) {msg("you can move again"); no_command = 0;}
    }
    else execcom();
    do_fuses();
    do_daemons();
    for (ntimes = LEFT; ntimes<=RIGHT; ntimes++) if (cur_ring[ntimes]) switch (cur_ring[ntimes]->o_which)
    {
      case R_SEARCH: search(); break;
      case R_TELEPORT: if (rnd(50)==17) teleport(); break;
    }
  }
}

com_char()
{
  int same, ch;

  same = (fastmode==faststate);
  ch = readchar();
  if (same) fastmode = faststate;
  else fastmode = !faststate;
  switch (ch)
  {
    case '\b': ch = 'h'; break;
    case '+': ch = 't'; break;
    case '-': ch = 'z'; break;
  }
  if (mpos && !running) msg("");
  return ch;
}

//Read a command, setting things up according to prefix like devices. Return the command character to be executed.
get_prefix()
{
  int retch, ch, junk;

  after = TRUE;
  fastmode = faststate;
  look(TRUE);
  if (!running) door_stop = FALSE;
  do_take = TRUE;
  again = FALSE;
  if (--count>0) {do_take = lasttake; retch = lastch; fastmode = FALSE;}
  else
  {
    count = 0;
    if (running) {retch = runch; do_take = lasttake;}
    else
    {
      for (retch = 0; retch==0;)
      {
        switch (ch = com_char())
        {
          case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            junk = count*10;
            if ((junk += ch-'0')>0 && junk<10000) count = junk;
            show_count();
          break;

          case 'f': fastmode = !fastmode; break;
          case 'g': do_take = FALSE; break;
          case 'a': retch = lastch; count = lastcount; do_take = lasttake; again = TRUE; break;
          case ' ': break;
          case ESCAPE: door_stop = FALSE; count = 0; show_count(); break;
          default: retch = ch; break;
        }
      }
    }
  }
  if (count) fastmode = FALSE;
  switch (retch)
  {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
      if (fastmode && !running)
      {
        if (!on(player, ISBLIND)) {door_stop = TRUE; firstmove = TRUE;}
        retch = toupper(retch);
      }

    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N': case 'q': case 'r': case 's': case 'z': case 't': case '.':

#ifdef WIZARD
    case CTRL('D'): case 'C':
#endif WIZARD

    break;

    default: count = 0;
  }
  if (count || lastcount) show_count();
  lastch = retch;
  lastcount = count;
  lasttake = do_take;
  return retch;
}

show_count()
{
  move(LINES-2, COLS-4);
  if (count) printw("%-4d", count);
  else addstr("    ");
}

execcom()
{
  coord mv;
  int ch;

  do
  {
    switch (ch = get_prefix())
    {
      case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n': find_dir(ch, &mv); do_move(mv.y, mv.x); break;
      case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N': do_run(tolower(ch)); break;
      case 't': if (get_dir()) missile(delta.y, delta.x); else after = FALSE; break;
      case 'Q': after = FALSE; quit(); break;
      case 'i': after = FALSE; inventory(pack, 0, ""); break;
      case 'd': drop(); break;
      case 'q': quaff(); break;
      case 'r': read_scroll(); break;
      case 'e': eat(); break;
      case 'w': wield(); break;
      case 'W': wear(); break;
      case 'T': take_off(); break;
      case 'P': ring_on(); break;
      case 'R': ring_off(); break;
      case 'c': after = FALSE; call(); break;
      case '>': after = FALSE; d_level(); break;
      case '<': after = FALSE; u_level(); break;
      case '/': after = FALSE; help(helpobjs); break;
      case '?': after = FALSE; help(helpcoms); break;
      case '!': after = FALSE; fakedos(); break;
      case 's': search(); break;
      case 'z': if (get_dir()) do_zap(); else after = FALSE; break;
      case 'D': after = FALSE; discovered(); break;
      case CTRL('T'): after = FALSE; msg((expert ^= 1)?"Ok, I'll be brief":"Goodie, I can use big words again!"); break;
      case 'F': after = FALSE; do_macro(macro, MACROSZ); break;
      case CTRL('F'): after = FALSE; typeahead = macro; break;
      case CTRL('R'): after = FALSE; msg(huh); break;

      case 'v':
        after = FALSE;
        if (strcmp(whoami, "The Grand Beeking")==0) addmsg("(%d)", csum());
        msg("Rogue version %d.%d (Mr. Mctesq was here)", revno, verno);
      break;

      case 'S': after = FALSE; save_game(); break;
      case '.': doctor(); break;

      case '^':
        after = FALSE;
        if (get_dir())
        {
          coord lookat;

          lookat.y = hero.y+delta.y;
          lookat.x = hero.x+delta.x;
          if (chat(lookat.y, lookat.x)!=TRAP) msg("no trap there.");
          else msg("you found %s", tr_name(flat(lookat.y, lookat.x)&F_TMASK));
        }
      break;

      case 'o': after = FALSE; msg("i don't have any options, oh my!"); break;
      case CTRL('L'): after = FALSE; msg("the screen looks fine to me (jll was here)"); break;

#ifdef WIZARD
      case 'C': after = FALSE; create_obj(); break;
#endif

      default: after = FALSE; save_msg = FALSE; msg("illegal command '%s'", unctrl(ch)); count = 0; save_msg = TRUE; break;
    }
    if (take && do_take) pick_up(take);
    take = 0;
    if (!running) door_stop = FALSE;
  } while (after==FALSE);
}
