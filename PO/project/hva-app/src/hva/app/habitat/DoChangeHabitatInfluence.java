package hva.app.habitat;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.exceptions.UnknownHabitatKeyException;
import hva.exceptions.UnknownHabitatException;
import hva.app.exceptions.UnknownSpeciesKeyException;
import hva.exceptions.UnknownSpeciesException;

import hva.exceptions.UnknownSpeciesException;

class DoChangeHabitatInfluence extends Command<Hotel> {

    DoChangeHabitatInfluence(Hotel receiver) {
        super(Label.CHANGE_HABITAT_INFLUENCE, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try {
            Form request = new Form();
            request.addStringField("habitatKey", Prompt.habitatKey());
            request.addStringField("speciesKey",
             hva.app.animal.Prompt.speciesKey());
            request.parse();

            String influence;
            do{
                influence = Form.requestString(
                    Prompt.habitatInfluence());
            } while (!influence.equals("POS") &&
             !influence.equals("NEG") && !influence.equals("NEU"));

            String habitatKey = request.stringField("habitatKey");
            String speciesKey = request.stringField("speciesKey");
            _receiver.changeHabitatInfluence(habitatKey, speciesKey,
             influence);
        } 
        catch (UnknownHabitatException e) {
            throw new UnknownHabitatKeyException(e.getKey());
        }
        catch (UnknownSpeciesException e) {
            throw new UnknownSpeciesKeyException(e.getKey());
        }

    }

}
