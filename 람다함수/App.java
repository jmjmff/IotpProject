package helloworld;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.TimeZone;

import com.amazonaws.services.dynamodbv2.AmazonDynamoDB;
import com.amazonaws.services.dynamodbv2.AmazonDynamoDBClientBuilder;
import com.amazonaws.services.dynamodbv2.document.DynamoDB;
import com.amazonaws.services.dynamodbv2.document.Item;
import com.amazonaws.services.dynamodbv2.document.PutItemOutcome;
import com.amazonaws.services.dynamodbv2.document.spec.PutItemSpec;
import com.amazonaws.services.dynamodbv2.model.ConditionalCheckFailedException;
import com.amazonaws.services.lambda.runtime.Context;
import com.amazonaws.services.lambda.runtime.RequestHandler;
import com.fasterxml.jackson.annotation.JsonCreator;

public class App implements RequestHandler<Thing, String> {

    private DynamoDB dynamoDb;
    private String DYNAMODB_TABLE_NAME = "wemos_table";

    @Override
    public String handleRequest(Thing input, Context context) {
        this.initDynamoDbClient();

        persistData(input);
        return "Success in storing to DB!";
    }

    private PutItemOutcome persistData(Thing thing) throws ConditionalCheckFailedException {

        // Epoch Conversion Code: https://www.epochconverter.com/
        SimpleDateFormat sdf = new SimpleDateFormat ( "yyyy-MM-dd HH:mm:ss");
        sdf.setTimeZone(TimeZone.getTimeZone("Asia/Seoul"));
        String timeString = sdf.format(new java.util.Date (thing.time*1000));

        return this.dynamoDb.getTable(DYNAMODB_TABLE_NAME)
                .putItem(new PutItemSpec().withItem(new Item()
                        .withPrimaryKey("device", thing.device)
                        .withLong("time", thing.time)
                        .withString("dust", thing.dust)
                        .withString("fire", thing.fire)
                        .withString("timestamp",timeString)));
    }

    private void initDynamoDbClient() {
        AmazonDynamoDB client = AmazonDynamoDBClientBuilder.standard().
                withRegion("ap-northeast-2").build();

        this.dynamoDb = new DynamoDB(client);
    }

}

class Thing {
    public String device;
    public long time;
    public String dust;
    public String fire;

}



