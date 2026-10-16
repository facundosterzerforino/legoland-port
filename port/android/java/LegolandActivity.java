package io.github.facundosterzerforino.legolandport;

import org.libsdl.app.SDLActivity;

/* SDL's activity, running the game. SDL is linked into the game's library (libmain.so), so that is the only one
 * to load. The game's command line, if any, comes in the launching intent's "args" extra (e.g.
 * adb shell am start -n <package>/.LegolandActivity --es args -port-selftest). */
public class LegolandActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {"main"};
    }

    @Override
    protected String[] getArguments() {
        String args = getIntent() != null ? getIntent().getStringExtra("args") : null;
        if (args == null || args.trim().isEmpty()) {
            return new String[0];
        }
        return args.trim().split("\\s+");
    }
}
