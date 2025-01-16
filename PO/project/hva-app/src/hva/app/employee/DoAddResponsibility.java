package hva.app.employee;

import hva.Hotel;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.CommandException;
import hva.exceptions.NoResponsabilityException;
import hva.exceptions.UnknownEmployeeException;
import hva.app.exceptions.UnknownEmployeeKeyException;

class DoAddResponsibility extends Command<Hotel> {

    DoAddResponsibility(Hotel receiver) {
        super(Label.ADD_RESPONSABILITY, receiver);
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
            _receiver.addResponsability(employeeKey, responsabilityKey
             );
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
