extern void obm();

void _start()
{
    obm();

    //if obm returns don't crash
    while (1);
}
