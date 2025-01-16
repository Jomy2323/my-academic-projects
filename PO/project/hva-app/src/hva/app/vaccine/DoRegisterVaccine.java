package hva.app.vaccine;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.exceptions.DuplicateVaccineKeyException;
import hva.exceptions.DuplicateVaccineException;
import hva.app.exceptions.UnknownSpeciesKeyException;
import hva.exceptions.UnknownSpeciesException;

class DoRegisterVaccine extends Command<Hotel> {

    DoRegisterVaccine(Hotel receiver) {
        super(Label.REGISTER_VACCINE, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
        try{
            Form request = new Form();
            request.addStringField("vaccineKey", Prompt.vaccineKey());
            request.addStringField("vaccineName", Prompt.vaccineName()
            );
            request.addStringField("listOfSpeciesKeys",
             Prompt.listOfSpeciesKeys());
            request.parse();

            String vaccineKey = request.stringField("vaccineKey");
            String vaccineName = request.stringField("vaccineName");
            String listOfSpeciesKeys = request.stringField
            ("listOfSpeciesKeys");
            _receiver.registerVaccine(new String[]{null, vaccineKey,
             vaccineName, listOfSpeciesKeys});
        }
        catch (DuplicateVaccineException e) {
            throw new DuplicateVaccineKeyException(e.getKey());
        }
        catch (UnknownSpeciesException e) {
            throw new UnknownSpeciesKeyException(e.getKey());
        }
    }

}
