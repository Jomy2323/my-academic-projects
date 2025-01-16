package hva.app.vaccine;

import hva.Hotel;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoShowVaccinations extends Command<Hotel> {

    DoShowVaccinations(Hotel receiver) {
        super(Label.SHOW_VACCINATIONS, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        _display.popup(_receiver.showAllVaccinations());
    }
}
