package hva.app.employee;

import hva.Hotel;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.CommandException;
import hva.exceptions.NoResponsabilityException;
import hva.exceptions.UnknownEmployeeException;
import hva.app.exceptions.UnknownEmployeeKeyException;

class DoRemoveResponsibility extends Command<Hotel> {

    DoRemoveResponsibility(Hotel receiver) {
        super(Label.REMOVE_RESPONSABILITY, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try{

            Form request = new Form();
            request.addStringField("employeeKey", Prompt.employeeKey
             ());
            request.addStringField("responsabilityKey", Prompt
             .responsibilityKey());
            request.parse();

            String employeeKey = request.stringField("employeeKey");
            String responsabilityKey = request.stringField
             ("responsabilityKey");

            _receiver.removeResponsability(employeeKey,
             responsabilityKey);
        }
        catch (UnknownEmployeeException e){
            throw new UnknownEmployeeKeyException(e.getKey());
        }
        catch (NoResponsabilityException e){
            throw new hva.app.exceptions.NoResponsibilityException(
                e.getEmployeeKey(), e.getResponsabilityKey());
        }
    }

}
