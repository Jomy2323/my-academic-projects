package hva.app.habitat;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.exceptions.UnknownHabitatKeyException;
import hva.exceptions.UnknownHabitatException;

class DoChangeHabitatArea extends Command<Hotel> {

    DoChangeHabitatArea(Hotel receiver) {
        super(Label.CHANGE_HABITAT_AREA, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try {
            Form request = new Form();
            request.addStringField("habitatKey", Prompt.habitatKey());
            request.addIntegerField("newArea", Prompt.habitatArea());
            request.parse();

            String habitatKey = request.stringField("habitatKey");
            int newArea = request.integerField("newArea");

            _receiver.changeHabitatArea(habitatKey, newArea);
        } catch (UnknownHabitatException e) {
            throw new UnknownHabitatKeyException(e.getKey());
        }
    }

}
