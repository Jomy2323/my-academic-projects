package hva.app.habitat;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.exceptions.DuplicateHabitatException;
import hva.app.exceptions.DuplicateHabitatKeyException;

class DoRegisterHabitat extends Command<Hotel> {

    DoRegisterHabitat(Hotel receiver) {
        super(Label.REGISTER_HABITAT, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try {
            Form request = new Form();
            request.addStringField("habitatKey", Prompt.habitatKey());
            request.addStringField("habitatName", Prompt.habitatName
             ());
            request.addStringField("habitatArea",Prompt.habitatArea
             ());
            request.parse();

            String habitatKey = request.stringField("habitatKey");
            String habitatName = request.stringField("habitatName");
            String habitatArea = request.stringField("habitatArea");    
            _receiver.registerHabitat(new String[] {null, habitatKey,
             habitatName, habitatArea});

        } catch (DuplicateHabitatException e) {
            throw new DuplicateHabitatKeyException(e.getKey());
        }
    }

}
