package hva.app.vaccine;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.exceptions.UnknownVeterinarianKeyException;
import hva.exceptions.UnknownVeterinarianException;
import hva.app.exceptions.UnknownVaccineKeyException;
import hva.exceptions.UnknownVaccineException;
import hva.app.exceptions.UnknownAnimalKeyException;
import hva.exceptions.UnknownAnimalException;
import hva.exceptions.VeterinarianNotAuthorizedException;

class DoVaccinateAnimal extends Command<Hotel> {

    DoVaccinateAnimal(Hotel receiver) {
        super(Label.VACCINATE_ANIMAL, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
        try{
            Form request = new Form();
            request.addStringField("vaccineKey", Prompt.vaccineKey());
            request.addStringField("vetKey",Prompt.veterinarianKey());
            request.addStringField("animalKey", hva.app.animal.Prompt.
             animalKey());
            request.parse();

            String vaccineKey = request.stringField("vaccineKey");
            String vetKey = request.stringField("vetKey");
            String animalKey = request.stringField("animalKey");

            boolean result = _receiver.vaccinateAnimal(vaccineKey,
             vetKey, animalKey);
            if (result)
                _display.popup(Message.wrongVaccine(vaccineKey, 
                 animalKey));
        }
        catch(UnknownVaccineException e){
            throw new UnknownVaccineKeyException(e.getKey());
        }
        catch(UnknownVeterinarianException e){
            throw new UnknownVeterinarianKeyException(e.getKey());
        }
        catch(UnknownAnimalException e){
            throw new UnknownAnimalKeyException(e.getKey());
        }
        catch(VeterinarianNotAuthorizedException e){
            throw new hva.app.exceptions
            .VeterinarianNotAuthorizedException(e.getVetKey(),
             e.getSpeciesKey());
        }
    }

}
