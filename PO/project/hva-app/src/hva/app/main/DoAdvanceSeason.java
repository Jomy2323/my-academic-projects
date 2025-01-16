package hva.app.main;

import hva.HotelManager;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoAdvanceSeason extends Command<HotelManager> {
    DoAdvanceSeason(HotelManager receiver) {
        super(Label.ADVANCE_SEASON, receiver);
    }

    @Override
    protected final void execute() {
       _display.popup(_receiver.getHotel().advanceSeason()); 
    }
}
